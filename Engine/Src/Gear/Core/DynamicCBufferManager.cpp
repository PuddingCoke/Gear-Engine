#include <Gear/Core/DynamicCBufferManager.h>

#include <Gear/Core/Internal/DynamicCBufferManagerInternal.h>

#include <Gear/Core/D3D12Resource/UploadHeap.h>

#include <Gear/Core/Graphics.h>

#include <Gear/Core/GlobalDescriptorHeap.h>

#include <Gear/Utils/Math.h>

#include <atomic>

namespace Gear::Core::DynamicCBufferManager
{
	class DynamicCBufferRegion
	{
	public:

		DynamicCBufferRegion(const uint64_t subRegionSize, const uint64_t subRegionNum);

		~DynamicCBufferRegion();

		uint32_t allocate();

		void retire(const uint32_t subRegionIndex);

		void* getUploadPtr(const uint32_t subRegionIndex) const;

		uint64_t getUpdateSize() const;

		uint64_t getSubRegionSize() const;

		D3D12Resource::UploadHeap* getUploadHeap() const;

	private:

		const uint64_t subRegionSize;

		const uint64_t subRegionNum;

		UniquePtr<UniquePtr<D3D12Resource::UploadHeap>[]> uploadHeap;

		UniquePtr<uint8_t*[]> dataPtr;

		std::vector<bool> occupied;

		uint32_t nextAllocateSearchStart;

		uint32_t updateSubRegionNum;

		std::mutex mutex;
	};

	DynamicCBufferRegion::DynamicCBufferRegion(const uint64_t subRegionSize, const uint64_t subRegionNum) :
		subRegionSize(subRegionSize),
		subRegionNum(subRegionNum),
		uploadHeap(makeUnique<UniquePtr<D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount())),
		dataPtr(makeUnique<uint8_t*[]>(Graphics::getFrameBufferCount())),
		occupied(subRegionNum, false),
		nextAllocateSearchStart(0),
		updateSubRegionNum(0)
	{
		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			uploadHeap[i] = makeUnique<D3D12Resource::UploadHeap>(subRegionSize * subRegionNum);

			//这里有个技巧，上传堆可以保持映射状态，这样可以避免每帧映射和取消映射带来的开销，但是不要忘了析构时取消映射
			dataPtr[i] = static_cast<uint8_t*>(uploadHeap[i]->map());
		}
	}

	DynamicCBufferRegion::~DynamicCBufferRegion()
	{
		if (uploadHeap.get())
		{
			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				if (uploadHeap[i].get())
				{
					uploadHeap[i]->unmap();
				}
			}
		}
	}

	uint32_t DynamicCBufferRegion::allocate()
	{
		std::lock_guard lock(mutex);

		uint32_t searchLocation = nextAllocateSearchStart;

		while (searchLocation < subRegionNum && occupied[searchLocation])
		{
			searchLocation++;
		}

		if (searchLocation == subRegionNum)
		{
			THROWLOG(LOGERROR() << "区域已满");
		}

		occupied[searchLocation] = true;

		nextAllocateSearchStart = searchLocation + 1;

		updateSubRegionNum = Utils::Math::max(updateSubRegionNum, searchLocation + 1);

		return searchLocation;
	}

	void DynamicCBufferRegion::retire(uint32_t subRegionIndex)
	{
		std::lock_guard lock(mutex);

		occupied[subRegionIndex] = false;

		nextAllocateSearchStart = Utils::Math::min(nextAllocateSearchStart, subRegionIndex);

		while (updateSubRegionNum > 0 && !occupied[updateSubRegionNum - 1])
		{
			--updateSubRegionNum;
		}
	}

	void* DynamicCBufferRegion::getUploadPtr(const uint32_t subRegionIndex) const
	{
		return dataPtr[Graphics::getFrameIndex()] + subRegionIndex * subRegionSize;
	}

	uint64_t DynamicCBufferRegion::getUpdateSize() const
	{
		return updateSubRegionNum * subRegionSize;
	}

	uint64_t DynamicCBufferRegion::getSubRegionSize() const
	{
		return subRegionSize;
	}

	D3D12Resource::UploadHeap* DynamicCBufferRegion::getUploadHeap() const
	{
		return uploadHeap[Graphics::getFrameIndex()].get();
	}

	class DynamicCBufferManagerImpl
	{
	public:

		DynamicCBufferManagerImpl();

		~DynamicCBufferManagerImpl();

		uint32_t requestLocation(const uint32_t regionIndex);

		void releaseLocation(const uint32_t regionIndex, const uint32_t subRegionIndex);

		D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress(const uint32_t regionIndex,
		                                        const uint32_t subRegionIndex) const;

		uint32_t getDescriptorIndex(const uint32_t regionIndex, const uint32_t subRegionIndex) const;

		void* getUploadPtr(const uint32_t regionIndex, const uint32_t subRegionIndex) const;

		void recordCommands(D3D12Core::GraphicsCommandList* const commandList);

		//每个区域的子区域数量
		//256bytes 512bytes 1024bytes .....
		static constexpr uint32_t numSubRegion[] = { 4096, 2048 };

		static constexpr uint32_t numRegion = _countof(numSubRegion);

	private:

		D3D12Resource::BufferPtr buffer;

		UniquePtr<UniquePtr<DynamicCBufferRegion>[]> bufferRegions;

		D3D12Core::DescriptorHandlePtr descriptorHandle;

		uint64_t dstOffset[numRegion];

		//constantBufferAddress = baseAddress[regionIndex] + (256 << regionIndex) * subregionIndex
		uint64_t baseAddress[numRegion];

		//descriptorIndex = baseDescriptorIndices[regionIndex] + subregionIndex
		uint32_t baseDescriptorIndices[numRegion];
	};

	DynamicCBufferManagerImpl::DynamicCBufferManagerImpl()
	{
		//创建一个大缓冲作为常量缓冲
		{
			uint32_t requiredSize = 0;

			for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
			{
				dstOffset[regionIndex] = requiredSize;

				requiredSize += (256u << regionIndex) * numSubRegion[regionIndex];
			}

			buffer = makeUnique<D3D12Resource::Buffer>(requiredSize, true, D3D12_RESOURCE_FLAG_NONE);

			buffer->setName(L"Large Constant Buffer");

			LOGSUCCESS() << "创建" << LogFloatPrecision(0) << requiredSize / 1024.f << "KB 的大常量缓冲";
		}

		//为大缓冲的每个区域创建上传堆，并进行管理
		{
			bufferRegions = makeUnique<UniquePtr<DynamicCBufferRegion>[]>(numRegion);

			for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
			{
				bufferRegions[regionIndex] =
					makeUnique<DynamicCBufferRegion>(256ull << regionIndex, numSubRegion[regionIndex]);
			}
		}

		//为每个区域的子区域创建CBV
		{
			uint32_t requiredDescriptorNum = 0;

			for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
			{
				requiredDescriptorNum += numSubRegion[regionIndex];
			}

			descriptorHandle =
				GlobalDescriptorHeap::getResourceHeap()->allocContiguousDescriptor(requiredDescriptorNum);

			uint64_t bufferLocationOffset = buffer->getGPUAddress();

			for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
			{
				baseAddress[regionIndex] = bufferLocationOffset;

				baseDescriptorIndices[regionIndex] = descriptorHandle->getCurrentIndex();

				for (uint32_t subregionIndex = 0; subregionIndex < numSubRegion[regionIndex];
				     subregionIndex++)
				{
					D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
					desc.BufferLocation = bufferLocationOffset;
					desc.SizeInBytes = (256u << regionIndex);

					bufferLocationOffset += (256ull << regionIndex);

					GraphicsDevice::get()->CreateConstantBufferView(&desc,
					                                                descriptorHandle->getCurrentCPUHandle());

					descriptorHandle->move();
				}
			}
		}
	}

	DynamicCBufferManagerImpl::~DynamicCBufferManagerImpl()
	{
	}

	uint32_t DynamicCBufferManagerImpl::requestLocation(const uint32_t regionIndex)
	{
		if (regionIndex >= numRegion)
		{
			std::string errorString = "动态常量缓冲的大小只能是以下这些";

			uint32_t factor = 1u;

			for (uint32_t i = 0; i < numRegion; i++)
			{
				errorString += " " + std::to_string(256u * factor) + "Bytes";

				factor <<= 1u;
			}

			THROWLOG(LOGERROR() << errorString);
		}

		return bufferRegions[regionIndex]->allocate();
	}

	void DynamicCBufferManagerImpl::releaseLocation(const uint32_t regionIndex, const uint32_t subRegionIndex)
	{
		bufferRegions[regionIndex]->retire(subRegionIndex);
	}

	D3D12_GPU_VIRTUAL_ADDRESS DynamicCBufferManagerImpl::getGPUAddress(const uint32_t regionIndex,
	                                                                   const uint32_t subRegionIndex) const
	{
		return baseAddress[regionIndex] + (256u << regionIndex) * subRegionIndex;
	}

	uint32_t DynamicCBufferManagerImpl::getDescriptorIndex(const uint32_t regionIndex,
	                                                       const uint32_t subRegionIndex) const
	{
		return baseDescriptorIndices[regionIndex] + subRegionIndex;
	}

	void* DynamicCBufferManagerImpl::getUploadPtr(const uint32_t regionIndex,
	                                              const uint32_t subRegionIndex) const
	{
		return bufferRegions[regionIndex]->getUploadPtr(subRegionIndex);
	}

	void DynamicCBufferManagerImpl::recordCommands(D3D12Core::GraphicsCommandList* const commandList)
	{
		//把大常量缓冲转变到STATE_COPY_DEST状态，用于内容更新
		commandList->trackAndSetResourceState(buffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->flushResourceBarriers();

		//划分有着固定子区域大小的区域后，可以通过极少的copyBufferRegion调用完成一帧所需的常量缓冲的更新，简直完美
		for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
		{
			if (bufferRegions[regionIndex]->getUpdateSize())
			{
				commandList->copyBufferRegion(buffer.get(),
				                              dstOffset[regionIndex],
				                              bufferRegions[regionIndex]->getUploadHeap(),
				                              0,
				                              bufferRegions[regionIndex]->getUpdateSize());
			}
		}

		//把大常量缓冲转变到STATE_VERTEX_AND_CONSTANT_BUFFER状态，用于后续使用
		commandList->trackAndSetResourceState(buffer.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		commandList->flushResourceBarriers();
	}

	UniquePtr<DynamicCBufferManagerImpl> impl;

	namespace Internal
	{
		void initialize()
		{
			impl = makeUnique<DynamicCBufferManagerImpl>();
		}

		void release()
		{
			impl.reset();
		}

		void recordCommands(D3D12Core::GraphicsCommandList* const commandList)
		{
			impl->recordCommands(commandList);
		}
	}

	DynamicCBufferManager::AvailableLocation::AvailableLocation(const uint32_t sizeParam) :
		regionIndex(checkAndGetRegionIndex(sizeParam)),
		subRegionIndex(impl->requestLocation(regionIndex)),
		size(256u << regionIndex)
	{
	}

	DynamicCBufferManager::AvailableLocation::~AvailableLocation()
	{
		impl->releaseLocation(regionIndex, subRegionIndex);
	}

	uint32_t DynamicCBufferManager::AvailableLocation::getRegionIndex() const
	{
		return regionIndex;
	}

	uint32_t DynamicCBufferManager::AvailableLocation::getSubRegionIndex() const
	{
		return subRegionIndex;
	}

	uint32_t DynamicCBufferManager::AvailableLocation::getSize() const
	{
		return size;
	}

	D3D12_GPU_VIRTUAL_ADDRESS DynamicCBufferManager::AvailableLocation::getGPUAddress() const
	{
		return impl->getGPUAddress(regionIndex, subRegionIndex);
	}

	uint32_t DynamicCBufferManager::AvailableLocation::getDescriptorIndex() const
	{
		return impl->getDescriptorIndex(regionIndex, subRegionIndex);
	}

	void* DynamicCBufferManager::AvailableLocation::getUploadPtr() const
	{
		return impl->getUploadPtr(regionIndex, subRegionIndex);
	}

	uint32_t DynamicCBufferManager::AvailableLocation::checkAndGetRegionIndex(const uint32_t sizeParam)
	{
		if (sizeParam == 0u)
		{
			THROWLOG(LOGERROR() << "动态常量缓冲的大小不能为0！");
		}

		return Utils::Math::log2Ceil((static_cast<uint64_t>(sizeParam) + 255ull) / 256ull);
	}

}

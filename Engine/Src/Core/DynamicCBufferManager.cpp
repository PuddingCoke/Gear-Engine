#include<Gear/Core/DynamicCBufferManager.h>

#include<Gear/Core/Internal/DynamicCBufferManagerInternal.h>

#include<Gear/Core/Resource/D3D12Resource/UploadHeap.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/GlobalDescriptorHeap.h>

#include<atomic>

namespace Gear::Core::DynamicCBufferManager::Internal
{
	class DynamicCBufferRegion
	{
	public:

		DynamicCBufferRegion(const uint64_t subRegionSize, const uint64_t subRegionNum);

		~DynamicCBufferRegion();

		struct Location
		{
			void* const dataPtr;
			const uint32_t subregionIndex;
		};

		Location acquireLocation();

		void reset();

		uint64_t getUpdateSize() const;

		Resource::D3D12Resource::UploadHeap* getUploadHeap() const;

	private:

		const uint64_t subRegionSize;

		Resource::D3D12Resource::UploadHeap** const uploadHeap;

		uint8_t** const dataPtr;

		std::atomic<uint64_t> currentOffset;

	};

	class DynamicCBufferManagerImpl
	{
	public:

		DynamicCBufferManagerImpl();

		~DynamicCBufferManagerImpl();

		DynamicCBufferManager::AvailableLocation requestLocation(const uint32_t regionIndex);

		void recordCommands(D3D12Core::CommandList* const commandList);

		//每个区域的子区域数量
		//256bytes 512bytes 1024bytes .....
		static constexpr uint32_t numSubRegion[] = { 4096,2048 };

		static constexpr uint32_t numRegion = sizeof(numSubRegion) / sizeof(uint32_t);

	private:

		Resource::D3D12Resource::Buffer* buffer;

		DynamicCBufferRegion** bufferRegions;

		uint64_t dstOffset[numRegion];

		//constantBufferAddress = baseAddressOffsets[regionIndex] + (256 << regionIndex) * subregionIndex
		uint64_t baseAddressOffsets[numRegion];

		//descriptorIndex = baseDescriptorIndices[regionIndex] + subregionIndex 
		uint32_t baseDescriptorIndices[numRegion];

	};

	DynamicCBufferRegion::DynamicCBufferRegion(const uint64_t subRegionSize, const uint64_t subRegionNum) :
		subRegionSize(subRegionSize),
		uploadHeap(new Resource::D3D12Resource::UploadHeap* [Graphics::getFrameBufferCount()]),
		dataPtr(new uint8_t* [Graphics::getFrameBufferCount()]),
		currentOffset(0)
	{
		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			uploadHeap[i] = new Resource::D3D12Resource::UploadHeap(subRegionSize * subRegionNum);

			dataPtr[i] = static_cast<uint8_t*>(uploadHeap[i]->map());
		}
	}

	DynamicCBufferRegion::~DynamicCBufferRegion()
	{
		if (uploadHeap)
		{
			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				if (uploadHeap[i])
				{
					uploadHeap[i]->unmap();

					delete uploadHeap[i];
				}
			}

			delete[] uploadHeap;
		}

		if (dataPtr)
		{
			delete[] dataPtr;
		}
	}

	DynamicCBufferRegion::Location DynamicCBufferRegion::acquireLocation()
	{
		const uint64_t tempOffset = currentOffset.fetch_add(subRegionSize, std::memory_order_relaxed);

		return { dataPtr[Graphics::getFrameIndex()] + tempOffset,static_cast<uint32_t>(tempOffset / subRegionSize) };
	}

	void DynamicCBufferRegion::reset()
	{
		currentOffset.store(0ull, std::memory_order_relaxed);
	}

	uint64_t DynamicCBufferRegion::getUpdateSize() const
	{
		return currentOffset.load(std::memory_order_relaxed);
	}

	Resource::D3D12Resource::UploadHeap* DynamicCBufferRegion::getUploadHeap() const
	{
		return uploadHeap[Graphics::getFrameIndex()];
	}

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

			buffer = new Resource::D3D12Resource::Buffer(requiredSize, true, D3D12_RESOURCE_FLAG_NONE);

			buffer->setName(L"Large Constant Buffer");

			LOGSUCCESS(L"create", FloatPrecision(0), requiredSize / 1024.f, L"kbytes constant buffer succeeded");
		}

		//为大缓冲的每个区域创建上传堆，并进行管理
		{
			bufferRegions = new DynamicCBufferRegion * [numRegion];

			for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
			{
				bufferRegions[regionIndex] = new DynamicCBufferRegion(256ull << regionIndex, numSubRegion[regionIndex]);
			}
		}

		//为每个区域的子区域创建CBV
		{
			uint32_t requiredDescriptorNum = 0;

			for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
			{
				requiredDescriptorNum += numSubRegion[regionIndex];
			}

			D3D12Core::DescriptorHandle descriptorHandle = GlobalDescriptorHeap::getResourceHeap()->allocStaticDescriptor(requiredDescriptorNum);

			uint64_t bufferLocationOffset = buffer->getGPUAddress();

			for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
			{
				baseAddressOffsets[regionIndex] = bufferLocationOffset;

				baseDescriptorIndices[regionIndex] = descriptorHandle.getCurrentIndex();

				for (uint32_t subregionIndex = 0; subregionIndex < numSubRegion[regionIndex]; subregionIndex++)
				{
					D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
					desc.BufferLocation = bufferLocationOffset;
					desc.SizeInBytes = (256u << regionIndex);

					bufferLocationOffset += (256ull << regionIndex);

					GraphicsDevice::get()->CreateConstantBufferView(&desc, descriptorHandle.getCurrentCPUHandle());

					descriptorHandle.move();
				}
			}
		}
	}

	DynamicCBufferManagerImpl::~DynamicCBufferManagerImpl()
	{
		if (buffer)
		{
			delete buffer;
		}

		if (bufferRegions)
		{
			for (uint32_t i = 0; i < numRegion; i++)
			{
				if (bufferRegions[i])
				{
					delete bufferRegions[i];
				}
			}

			delete[] bufferRegions;
		}
	}

	DynamicCBufferManager::AvailableLocation DynamicCBufferManagerImpl::requestLocation(const uint32_t regionIndex)
	{
		const DynamicCBufferRegion::Location location = bufferRegions[regionIndex]->acquireLocation();

		return { location.dataPtr,baseAddressOffsets[regionIndex] + (256u << regionIndex) * location.subregionIndex,baseDescriptorIndices[regionIndex] + location.subregionIndex };
	}

	void DynamicCBufferManagerImpl::recordCommands(D3D12Core::CommandList* const commandList)
	{
		//把大常量缓冲转变到STATE_COPY_DEST状态，用于内容更新
		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->transitionResources();

		for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
		{
			if (bufferRegions[regionIndex]->getUpdateSize())
			{
				commandList->copyBufferRegion(buffer, dstOffset[regionIndex],
					bufferRegions[regionIndex]->getUploadHeap(), 0,
					bufferRegions[regionIndex]->getUpdateSize());

				bufferRegions[regionIndex]->reset();
			}
		}

		//把大常量缓冲转变到STATE_VERTEX_AND_CONSTANT_BUFFER状态，用于后续使用
		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		commandList->transitionResources();
	}
}

namespace
{
	UniquePtr<Gear::Core::DynamicCBufferManager::Internal::DynamicCBufferManagerImpl> impl;
}

uint32_t Gear::Core::DynamicCBufferManager::getNumRegion()
{
	return Gear::Core::DynamicCBufferManager::Internal::DynamicCBufferManagerImpl::numRegion;
}

Gear::Core::DynamicCBufferManager::AvailableLocation Gear::Core::DynamicCBufferManager::requestLocation(const uint32_t regionIndex)
{
	return impl->requestLocation(regionIndex);
}

void Gear::Core::DynamicCBufferManager::Internal::initialize()
{
	impl = makeUnique<DynamicCBufferManagerImpl>();
}

void Gear::Core::DynamicCBufferManager::Internal::release()
{
	impl.reset();
}

void Gear::Core::DynamicCBufferManager::Internal::recordCommands(Gear::Core::D3D12Core::CommandList* const commandList)
{
	impl->recordCommands(commandList);
}

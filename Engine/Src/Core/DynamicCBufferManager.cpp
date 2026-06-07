#include<Gear/Core/DynamicCBufferManager.h>

#include<Gear/Core/Internal/DynamicCBufferManagerInternal.h>

#include<Gear/Core/Resource/D3D12Resource/UploadHeap.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/GlobalDescriptorHeap.h>

#include<atomic>

namespace Gear::Core::DynamicCBufferManager
{
	namespace Internal
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

			UniquePtr<UniquePtr<Resource::D3D12Resource::UploadHeap>[]> uploadHeap;

			UniquePtr<uint8_t* []> dataPtr;

			std::atomic<uint64_t> currentOffset;

		};

		DynamicCBufferRegion::DynamicCBufferRegion(const uint64_t subRegionSize, const uint64_t subRegionNum) :
			subRegionSize(subRegionSize),
			uploadHeap(makeUnique<UniquePtr<Resource::D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount())),
			dataPtr(makeUnique<uint8_t* []>(Graphics::getFrameBufferCount())),
			currentOffset(0)
		{
			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				uploadHeap[i] = makeUnique<Resource::D3D12Resource::UploadHeap>(subRegionSize * subRegionNum);

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
			return uploadHeap[Graphics::getFrameIndex()].get();
		}

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

			UniquePtr<Resource::D3D12Resource::Buffer> buffer;

			UniquePtr<UniquePtr<DynamicCBufferRegion>[]> bufferRegions;

			uint64_t dstOffset[numRegion];

			//constantBufferAddress = baseAddressOffsets[regionIndex] + (256 << regionIndex) * subregionIndex
			uint64_t baseAddressOffsets[numRegion];

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

				buffer = makeUnique<Resource::D3D12Resource::Buffer>(requiredSize, true, D3D12_RESOURCE_FLAG_NONE);

				buffer->setName(L"Large Constant Buffer");

				LOGSUCCESS(L"创建", FloatPrecision(0), requiredSize / 1024.f, L"KB 的大常量缓冲");
			}

			//为大缓冲的每个区域创建上传堆，并进行管理
			{
				bufferRegions = makeUnique<UniquePtr<DynamicCBufferRegion>[]>(numRegion);

				for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
				{
					bufferRegions[regionIndex] = makeUnique<DynamicCBufferRegion>(256ull << regionIndex, numSubRegion[regionIndex]);
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
		}

		DynamicCBufferManager::AvailableLocation DynamicCBufferManagerImpl::requestLocation(const uint32_t regionIndex)
		{
			const DynamicCBufferRegion::Location location = bufferRegions[regionIndex]->acquireLocation();

			return { location.dataPtr,baseAddressOffsets[regionIndex] + (256u << regionIndex) * location.subregionIndex,baseDescriptorIndices[regionIndex] + location.subregionIndex };
		}

		void DynamicCBufferManagerImpl::recordCommands(D3D12Core::CommandList* const commandList)
		{
			//把大常量缓冲转变到STATE_COPY_DEST状态，用于内容更新
			commandList->trackAndSetResourceState(buffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);

			commandList->transitionResources();

			//划分有着固定子区域大小的区域后，可以通过极少的copyBufferRegion调用完成一帧所需的常量缓冲的更新，简直完美
			for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
			{
				if (bufferRegions[regionIndex]->getUpdateSize())
				{
					commandList->copyBufferRegion(buffer.get(), dstOffset[regionIndex],
						bufferRegions[regionIndex]->getUploadHeap(), 0,
						bufferRegions[regionIndex]->getUpdateSize());

					bufferRegions[regionIndex]->reset();
				}
			}

			//把大常量缓冲转变到STATE_VERTEX_AND_CONSTANT_BUFFER状态，用于后续使用
			commandList->trackAndSetResourceState(buffer.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

			commandList->transitionResources();
		}

		UniquePtr<DynamicCBufferManagerImpl> impl;

		void initialize()
		{
			impl = makeUnique<DynamicCBufferManagerImpl>();
		}

		void release()
		{
			impl.reset();
		}

		void recordCommands(D3D12Core::CommandList* const commandList)
		{
			impl->recordCommands(commandList);
		}
	}

	uint32_t DynamicCBufferManager::getNumRegion()
	{
		return Internal::DynamicCBufferManagerImpl::numRegion;
	}

	DynamicCBufferManager::AvailableLocation DynamicCBufferManager::requestLocation(const uint32_t regionIndex)
	{
		return Internal::impl->requestLocation(regionIndex);
	}
}

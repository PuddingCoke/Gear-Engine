#include <Gear/Core/DynamicCBufferManager.h>

#include <Gear/Core/Internal/DynamicCBufferManagerInternal.h>

#include <Gear/Core/D3D12Resource/UploadHeap.h>

#include <Gear/Core/Graphics.h>

#include <Gear/Core/GlobalDescriptorHeap.h>

#include <Gear/Utils/Math.h>

namespace Gear::Core::DynamicCBufferManager
{
	class DynamicCBufferRegion
	{
	public:

		DynamicCBufferRegion(const uint64_t subRegionSize, const uint64_t numSubRegion);

		~DynamicCBufferRegion();

		uint32_t allocate();

		void retire(const uint32_t subRegionIndex);

		void* getUploadPtr(const uint32_t subRegionIndex) const;

		uint64_t getUpdateSize() const;

		uint64_t getSubRegionSize() const;

		D3D12Resource::UploadHeap* getUploadHeap() const;

		void updateMappedData();

	private:

		void copyRun(const uint8_t* const srcPtr, uint8_t* const dstPtr, const uint32_t runBegin,
		             const uint32_t runEnd) const;

		//两级位图：第一级每个槽位1位(1表示空闲)，第二级每个第一级的字1位(1表示该字里还有空闲槽位)
		//8192个槽位只需要128个第一级字和2个第二级字，分配和回收都是常数时间，且总能取到最低的空闲槽位
		static constexpr uint32_t maxNumSubRegion = 8192;

		static constexpr uint32_t numWord = maxNumSubRegion / 64;

		static constexpr uint32_t numMaxSummaryWord = numWord / 64;

		const uint64_t subRegionSize;

		const uint64_t numSubRegion;

		UniquePtr<UniquePtr<D3D12Resource::UploadHeap>[]> uploadHeap;

		UniquePtr<uint8_t*[]> mappedDataPtr;

		UniquePtr<uint8_t[]> shadowData;

		uint64_t freeMask[numWord];

		uint64_t freeSummary[numMaxSummaryWord];

		//实际用到的第一级字数量、第二级字数量，以及最后一个字中有效槽位的掩码(尾部无效位保持为0表示已占用)
		uint32_t numUsedWord;

		uint32_t numUsedSummaryWord;

		uint64_t lastWordMask;

		uint32_t numUpdateSubRegion;

		std::mutex mutex;
	};

	DynamicCBufferRegion::DynamicCBufferRegion(const uint64_t subRegionSize, const uint64_t numSubRegion) :
		subRegionSize(subRegionSize),
		numSubRegion(numSubRegion),
		uploadHeap(makeUnique<UniquePtr<D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount())),
		mappedDataPtr(makeUnique<uint8_t*[]>(Graphics::getFrameBufferCount())),
		shadowData(makeUnique<uint8_t[]>(subRegionSize * numSubRegion)),
		freeMask{},
		freeSummary{},
		numUsedWord(0),
		numUsedSummaryWord(0),
		lastWordMask(~0ull),
		numUpdateSubRegion(0)
	{
		if (numSubRegion == 0ull || numSubRegion > maxNumSubRegion)
		{
			THROWLOG(LOGERROR() << "子区域数量必须在1到" << maxNumSubRegion << "之间");
		}

		numUsedWord = static_cast<uint32_t>((numSubRegion + 63ull) / 64ull);

		numUsedSummaryWord = (numUsedWord + 63u) / 64u;

		const uint32_t lastWordBits = static_cast<uint32_t>(numSubRegion & 63ull);

		lastWordMask = lastWordBits ? ((1ull << lastWordBits) - 1ull) : ~0ull;

		for (uint32_t wordIndex = 0; wordIndex < numUsedWord; wordIndex++)
		{
			freeMask[wordIndex] = (wordIndex + 1u == numUsedWord) ? lastWordMask : ~0ull;

			freeSummary[wordIndex >> 6] |= 1ull << (wordIndex & 63);
		}

		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			uploadHeap[i] = makeUnique<D3D12Resource::UploadHeap>(subRegionSize * numSubRegion);

			//这里有个技巧，上传堆可以保持映射状态，这样可以避免每帧映射和取消映射带来的开销，但是不要忘了析构时取消映射
			mappedDataPtr[i] = static_cast<uint8_t*>(uploadHeap[i]->map());
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

		//第二级只有一到两个字，这里是最多两次判定
		uint32_t summaryIndex = 0;

		while (summaryIndex < numUsedSummaryWord && freeSummary[summaryIndex] == 0ull)
		{
			summaryIndex++;
		}

		if (summaryIndex == numUsedSummaryWord)
		{
			THROWLOG(LOGERROR() << "区域已满");
		}

		const uint32_t wordIndex =
			(summaryIndex << 6) + Utils::Math::bitScanForward(freeSummary[summaryIndex]);

		const uint64_t mask = freeMask[wordIndex];

		//取最低的空闲槽位，保证已占用的最高索引尽可能低，从而减少每帧的拷贝量
		const uint32_t subRegionIndex = (wordIndex << 6) + Utils::Math::bitScanForward(mask);

		const uint64_t remainingMask = mask & (mask - 1ull);

		freeMask[wordIndex] = remainingMask;

		if (remainingMask == 0ull)
		{
			freeSummary[summaryIndex] &= ~(1ull << (wordIndex & 63));
		}

		return subRegionIndex;
	}

	void DynamicCBufferRegion::retire(const uint32_t subRegionIndex)
	{
		if (subRegionIndex >= numSubRegion)
		{
			THROWLOG(LOGERROR() << "非法的子区域索引");
		}

		std::lock_guard lock(mutex);

		const uint32_t wordIndex = subRegionIndex >> 6;

		const uint64_t previousMask = freeMask[wordIndex];

		freeMask[wordIndex] = previousMask | (1ull << (subRegionIndex & 63));

		if (previousMask == 0ull)
		{
			freeSummary[wordIndex >> 6] |= 1ull << (wordIndex & 63);
		}
	}

	void* DynamicCBufferRegion::getUploadPtr(const uint32_t subRegionIndex) const
	{
		return shadowData.get() + subRegionIndex * subRegionSize;
	}

	void DynamicCBufferRegion::updateMappedData()
	{
		std::lock_guard lock(mutex);

		uint8_t* const dstPtr = mappedDataPtr[Graphics::getFrameBufferIndex()];

		const uint8_t* const srcPtr = shadowData.get();

		//待处理的连续区间[runBegin, runEnd)
		uint32_t runBegin = 0;

		uint32_t runEnd = 0;

		for (uint32_t wordIndex = 0; wordIndex < numUsedWord; wordIndex++)
		{
			//freeMask里1表示空闲，取反就得到"已创建"的槽位
			uint64_t mask = ~freeMask[wordIndex];

			if (mask == 0ull)
			{
				continue;
			}

			if (wordIndex + 1u == numUsedWord)
			{
				mask &= lastWordMask;
			}

			while (mask)
			{
				const uint32_t bitIndex = Utils::Math::bitScanForward(mask);

				const uint32_t runBeginCandidate = (wordIndex << 6) + bitIndex;

				//从bitIndex开始连续1的数量
				const uint64_t rest = mask >> bitIndex;

				const uint32_t runLength = (rest == ~0ull)
					? (64u - bitIndex)
					: Utils::Math::bitScanForward(~rest);

				const uint32_t runEndCandidate = runBeginCandidate + runLength;

				//与上一个区间相邻(含跨字)，直接合并，稠密时就能退化成一次大拷贝
				if (runEnd == runBeginCandidate)
				{
					runEnd = runEndCandidate;
				}
				else
				{
					copyRun(srcPtr, dstPtr, runBegin, runEnd);

					runBegin = runBeginCandidate;

					runEnd = runEndCandidate;
				}

				//只清掉这一段的位，更高位上的区间留给下一次循环
				const uint64_t runMask = (runLength == 64u) ? ~0ull : ((1ull << runLength) - 1ull);

				mask &= ~(runMask << bitIndex);
			}
		}

		copyRun(srcPtr, dstPtr, runBegin, runEnd);

		//已创建的最高槽位+1，直接供recordCommands里那条copyBufferRegion使用
		numUpdateSubRegion = runEnd;
	}

	void DynamicCBufferRegion::copyRun(const uint8_t* const srcPtr, uint8_t* const dstPtr,
	                                   const uint32_t runBegin, const uint32_t runEnd) const
	{
		if (runBegin == runEnd)
		{
			return;
		}

		memcpy(dstPtr + runBegin * subRegionSize,
		       srcPtr + runBegin * subRegionSize,
		       static_cast<size_t>(runEnd - runBegin) * subRegionSize);
	}

	uint64_t DynamicCBufferRegion::getUpdateSize() const
	{
		return numUpdateSubRegion * subRegionSize;
	}

	uint64_t DynamicCBufferRegion::getSubRegionSize() const
	{
		return subRegionSize;
	}

	D3D12Resource::UploadHeap* DynamicCBufferRegion::getUploadHeap() const
	{
		return uploadHeap[Graphics::getFrameBufferIndex()].get();
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
		for (uint32_t regionIndex = 0; regionIndex < numRegion; regionIndex++)
		{
			bufferRegions[regionIndex]->updateMappedData();
		}

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
		size(256u << regionIndex),
		uploadPtr(impl->getUploadPtr(regionIndex, subRegionIndex))
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
		return uploadPtr;
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

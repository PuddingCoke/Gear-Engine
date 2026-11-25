#include <Gear/Resource/DynamicCBuffer.h>

#include <Gear/Core/DynamicCBufferManager.h>

#include <Gear/Core/Graphics.h>

#include <Gear/Utils/Math.h>

namespace Gear::Resource
{
	DynamicCBuffer::DynamicCBuffer(const uint32_t sizeParam) :
		ImmutableCBuffer(nullptr),
		regionIndex(Utils::Math::log2Ceil((static_cast<uint64_t>(sizeParam) + 255ull) / 256ull)),
		size(static_cast<uint32_t>(256ull << regionIndex)),
		dataPtr(makeShared<uint8_t[]>(size)),
		mappedDataPtr(makeShared<void*>(nullptr)),
		updateDataPtrFrameEpoch(makeShared<uint64_t>(UINT64_MAX)),
		acquireFrameEpoch(makeShared<uint64_t>(UINT64_MAX)),
		updateMappedDataPtrFrameEpoch(makeShared<uint64_t>(UINT64_MAX))
	{
		if (sizeParam == 0)
		{
			THROWLOG(LOGERROR() << "动态常量缓冲的大小不能为0");
		}

		if (regionIndex >= DynamicCBufferManager::getNumRegion())
		{
			std::string errorString = "动态常量缓冲的大小只能是以下这些";

			uint32_t factor = 1u;

			for (uint32_t i = 0; i < DynamicCBufferManager::getNumRegion(); i++)
			{
				errorString += " " + std::to_string(256u * factor) + "Bytes";

				factor <<= 1u;
			}

			THROWLOG(LOGERROR() << errorString);
		}
	}

	DynamicCBuffer::DynamicCBuffer(const DynamicCBuffer& dcb) :
		ImmutableCBuffer(dcb),
		regionIndex(dcb.regionIndex),
		size(dcb.size),
		dataPtr(dcb.dataPtr),
		mappedDataPtr(dcb.mappedDataPtr),
		updateDataPtrFrameEpoch(dcb.updateDataPtrFrameEpoch),
		acquireFrameEpoch(dcb.acquireFrameEpoch),
		updateMappedDataPtrFrameEpoch(dcb.updateMappedDataPtrFrameEpoch)
	{
	}

	void DynamicCBuffer::update(const void* const data, const uint32_t updateSize)
	{
#ifdef _DEBUG
		if (Graphics::getFrameEpoch() == *updateDataPtrFrameEpoch)
		{
			THROWLOG(LOGERROR() << "一个动态常量缓冲每帧只能更新一次！");
		}

		if (updateSize > size)
		{
			THROWLOG(LOGERROR() << "更新大小不能大于动态常量缓冲的大小！");
		}
#endif // _DEBUG

		*updateDataPtrFrameEpoch = Graphics::getFrameEpoch();

		memcpy(dataPtr.get(), data, updateSize);
	}

	bool DynamicCBuffer::hasVolatileDescriptor() const
	{
		return true;
	}

	void DynamicCBuffer::acquireMappedDataPtr()
	{
#ifdef _DEBUG
		if (*acquireFrameEpoch == Graphics::getFrameEpoch())
		{
			THROWLOG(LOGERROR() << "对于动态常量缓冲来说，每帧的数据指针获取只能有一次！");
		}
#endif // _DEBUG

		*acquireFrameEpoch = Graphics::getFrameEpoch();

		const DynamicCBufferManager::AvailableLocation& location =
			DynamicCBufferManager::requestLocation(regionIndex);

		*mappedDataPtr = location.dataPtr;

		*gpuAddress = location.gpuAddress;

		*bufferIndex = location.descriptorIndex;
	}

	void DynamicCBuffer::updateMappedData()
	{
#ifdef _DEBUG
		if(*acquireFrameEpoch != Graphics::getFrameEpoch())
		{
			THROWLOG(LOGERROR() << "对于动态常量缓冲来说，必须先获取数据指针才能更新映射数据！");
		}

		if (*updateMappedDataPtrFrameEpoch == Graphics::getFrameEpoch())
		{
			THROWLOG(LOGERROR() << "对于动态常量缓冲来说，每帧的更新只能有一次！");
		}
#endif // _DEBUG

		*updateMappedDataPtrFrameEpoch = Graphics::getFrameEpoch();

		memcpy(*mappedDataPtr, dataPtr.get(), size);
	}

	void DynamicCBuffer::prepareDescriptorsImpl()
	{
		acquireMappedDataPtr();

		updateMappedData();
	}

}

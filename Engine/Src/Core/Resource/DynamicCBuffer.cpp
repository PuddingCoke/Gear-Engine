#include<Gear/Core/Resource/DynamicCBuffer.h>

#include<Gear/Core/DynamicCBufferManager.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Utils/Logger.h>

#include<Gear/Utils/Math.h>

namespace Gear::Core::Resource
{
	DynamicCBuffer::DynamicCBuffer(const uint32_t size) :
		ImmutableCBuffer(nullptr, size, true), regionIndex(Utils::Math::log2(size / 256u)), dataPtr(nullptr), acquireFrameIndex(UINT64_MAX), updateFrameIndex(UINT64_MAX)
	{
		if (regionIndex >= DynamicCBufferManager::getNumRegion())
		{
			std::wstring errorString = L"动态常量缓冲的大小只能是以下这些";

			uint32_t factor = 1u;

			for (uint32_t i = 0; i < DynamicCBufferManager::getNumRegion(); i++)
			{
				errorString += L" " + std::to_wstring(256u * factor) + L"Bytes";

				factor <<= 1u;
			}

			LOGERROR(errorString);
		}
	}

	void DynamicCBuffer::acquireDataPtr()
	{
#ifdef _DEBUG
		if (acquireFrameIndex == Graphics::getRenderedFrameCount())
		{
			LOGERROR(L"对于动态常量缓冲来说，每帧的数据指针获取只能有一次！");
		}
#endif // _DEBUG

		acquireFrameIndex = Graphics::getRenderedFrameCount();

		const DynamicCBufferManager::AvailableLocation& location = DynamicCBufferManager::requestLocation(regionIndex);

		dataPtr = location.dataPtr;

		gpuAddress = location.gpuAddress;

		bufferIndex = location.descriptorIndex;
	}

	void DynamicCBuffer::updateData(const void* const data)
	{
#ifdef _DEBUG
		if (acquireFrameIndex != Graphics::getRenderedFrameCount())
		{
			LOGERROR(L"你还没有为这个动态常量缓冲获取这一帧可用的数据指针！");
		}

		if (updateFrameIndex == Graphics::getRenderedFrameCount())
		{
			LOGERROR(L"一个动态常量缓冲每帧只能更新一次！");
		}
#endif // _DEBUG

		updateFrameIndex = Graphics::getRenderedFrameCount();

		memcpy(dataPtr, data, 256ull << regionIndex);
	}

	void DynamicCBuffer::simpleUpdate(const void* const data)
	{
		acquireDataPtr();

		updateData(data);
	}
}
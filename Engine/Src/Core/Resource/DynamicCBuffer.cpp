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
			std::wstring errorString = L"dynamic constant buffer size should be one of";

			uint32_t factor = 1u;

			for (uint32_t i = 0; i < DynamicCBufferManager::getNumRegion(); i++)
			{
				errorString += L" " + std::to_wstring(256u * factor) + L"bytes";

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
			LOGERROR(L"you can only acquire data pointer for cbuffer once per frame");
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
			LOGERROR(L"you haven't acquire data pointer for this dynamic cbuffer yet");
		}

		if (updateFrameIndex == Graphics::getRenderedFrameCount())
		{
			LOGERROR(L"you can only update cbuffer once per frame");
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
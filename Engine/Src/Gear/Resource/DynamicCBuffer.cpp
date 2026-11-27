#include <Gear/Resource/DynamicCBuffer.h>

#include <Gear/Core/DynamicCBufferManager.h>

namespace Gear::Resource
{
	DynamicCBuffer::DynamicCBuffer(const uint32_t size) :
		ImmutableCBuffer(nullptr),
		location(makeShared<DynamicCBufferManager::AvailableLocation>(size))
	{
		gpuAddress = location->getGPUAddress();

		bufferIndex = location->getDescriptorIndex();
	}

	DynamicCBuffer::DynamicCBuffer(const DynamicCBuffer& dcb) :
		ImmutableCBuffer(dcb), location(dcb.location)
	{
	}

	DynamicCBuffer::~DynamicCBuffer()
	{
	}

	void DynamicCBuffer::update(const void* const data, const uint32_t updateSize)
	{
		if (updateSize > location->getSize())
		{
			THROWLOG(LOGERROR() << "更新大小不能大于动态常量缓冲的大小！");
		}

		memcpy(location->getUploadPtr(), data, updateSize);
	}

	void* DynamicCBuffer::getDataPtr() const
	{
		return location->getUploadPtr();
	}

	uint32_t DynamicCBuffer::getSize() const
	{
		return location->getSize();
	}

}

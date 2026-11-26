#include <Gear/Resource/DynamicCBuffer.h>

#include <Gear/Core/DynamicCBufferManager.h>

#include <Gear/Core/Graphics.h>

#include <Gear/Utils/Math.h>

namespace Gear::Resource
{
	DynamicCBuffer::DynamicCBuffer(const uint32_t size) :
		ImmutableCBuffer(nullptr),
		location(makeShared<DynamicCBufferManager::AvailableLocation>(size)),
		updateMutex(makeShared<std::mutex>()),
		updateFrameEpoch(makeShared<uint64_t>(UINT64_MAX))
	{
		gpuAddress = location->getGPUAddress();

		bufferIndex = location->getDescriptorIndex();
	}

	DynamicCBuffer::DynamicCBuffer(const DynamicCBuffer& dcb) :
		ImmutableCBuffer(dcb),
		location(dcb.location),
		updateMutex(dcb.updateMutex),
		updateFrameEpoch(dcb.updateFrameEpoch)
	{
	}

	DynamicCBuffer::~DynamicCBuffer()
	{
	}

	#pragma message("TODO: 这里检查动态常量缓冲每帧是否都更新的代码不妥当")
	void DynamicCBuffer::update(const void* const data, const uint32_t updateSize)
	{
		if (updateSize > location->getSize())
		{
			THROWLOG(LOGERROR() << "更新大小不能大于动态常量缓冲的大小！");
		}

		std::lock_guard<std::mutex> lock(*updateMutex);

		//跳过初始化和第一帧，检测每帧是否都更新了
		if (*updateFrameEpoch != 0ull && *updateFrameEpoch != UINT64_MAX &&
		    Graphics::getFrameEpoch() - *updateFrameEpoch > 1)
		{
			THROWLOG(LOGERROR() << "动态常量缓冲跳过了 "
			                    << (Graphics::getFrameEpoch() - *updateFrameEpoch - 1) << " 帧未更新");
		}

		if (*updateFrameEpoch == Graphics::getFrameEpoch())
		{
			THROWLOG(LOGERROR() << "对于动态常量缓冲来说，每帧的更新只能有一次！");
		}

		*updateFrameEpoch = Graphics::getFrameEpoch();

		memcpy(location->getUploadPtr(), data, updateSize);
	}

	uint32_t DynamicCBuffer::getSize() const
	{
		return location->getSize();
	}

}

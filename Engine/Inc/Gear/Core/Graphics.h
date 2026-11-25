#pragma once

#ifndef _GEAR_CORE_GRAPHICS_H_
#define _GEAR_CORE_GRAPHICS_H_

#include <Gear/Core/FMT.h>

namespace Gear::Core::Graphics
{
	float getExposure();

	void setExposure(const float exposure);

	float getGamma();

	void setGamma(const float gamma);

	uint32_t getFrameBufferCount();

	uint32_t getFrameIndex();

	float getDeltaTime();

	float getTimeElapsed();

	float getFrameRate();

	uint32_t getWidth();

	uint32_t getHeight();

	float getAspectRatio();

	//已经渲染的帧数
	uint64_t getRenderedFrameCount();

	//当前帧的epoch 初始化为0，渲染第一帧时从1开始，每帧完成后自增1
	uint64_t getFrameEpoch();

	//据说把后背缓冲设置成FMT::BGRA8UN格式会有更好的性能
	constexpr DXGI_FORMAT backBufferFormat = FMT::BGRA8UN;
}

#endif // !_GEAR_CORE_GRAPHICS_H_

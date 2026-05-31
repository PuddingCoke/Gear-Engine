#pragma once

#ifndef _GEAR_CORE_GRAPHICS_H_
#define _GEAR_CORE_GRAPHICS_H_

#include<Gear/Core/FMT.h>

#include<Gear/Core/Resource/ImmutableCBuffer.h>

namespace Gear
{
	namespace Core
	{
		namespace Graphics
		{

			float getExposure();

			void setExposure(const float exposure);

			float getGamma();

			void setGamma(const float gamma);

			uint32_t getFrameBufferCount();

			uint32_t getFrameIndex();

			float getDeltaTime();

			float getTimeElapsed();

			uint32_t getWidth();

			uint32_t getHeight();

			float getAspectRatio();

			uint64_t getRenderedFrameCount();

			D3D12_CPU_DESCRIPTOR_HANDLE getBackBufferHandle();

			Resource::ImmutableCBuffer* getEngineDefinedGlobalCBuffer();

			//据说把后背缓冲设置成FMT::BGRA8UN格式会有更好的性能
			constexpr DXGI_FORMAT backBufferFormat = FMT::BGRA8UN;

		}
	}
}

#endif // !_GEAR_CORE_GRAPHICS_H_
#pragma once

#ifndef _GEAR_CORE_RESOURCE_SWAPTEXTURE_H_
#define _GEAR_CORE_RESOURCE_SWAPTEXTURE_H_

#include"RenderTextureView.h"

#include"SwappableBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(SwapTexture);

	class SwapTexture :public SwappableBase<RenderTextureView>
	{
	public:

		SwapTexture() = delete;

		SwapTexture(const std::function<RenderTextureViewPtr(void)>& readTextureFunc, const std::function<RenderTextureViewPtr(void)>& writeTextureFunc);

		~SwapTexture();

		const uint32_t width;

		const uint32_t height;

	};
}

#endif // !_GEAR_CORE_RESOURCE_SWAPTEXTURE_H_
#pragma once

#ifndef _GEAR_CORE_RESOURCE_SWAPTEXTURE_H_
#define _GEAR_CORE_RESOURCE_SWAPTEXTURE_H_

#include"TextureRenderView.h"

#include"SwappableBase.h"

namespace Gear
{
	namespace Core
	{
		namespace Resource
		{
			class SwapTexture :public SwappableBase<TextureRenderView>
			{
			public:

				SwapTexture() = delete;

				SwapTexture(const std::function<TextureRenderView* (void)>& readTextureFunc, const std::function<TextureRenderView* (void)>& writeTextureFunc);

				SwapTexture(const std::function<TextureRenderView* (void)>& textureFunc);

				~SwapTexture();

				const uint32_t width;

				const uint32_t height;

			};
		}
	}
}

#endif // !_GEAR_CORE_RESOURCE_SWAPTEXTURE_H_
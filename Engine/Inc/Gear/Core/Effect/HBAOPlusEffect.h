#pragma once

#ifndef _GEAR_CORE_EFFECT_HBAOPLUSEFFECT_H_
#define _GEAR_CORE_EFFECT_HBAOPLUSEFFECT_H_

#include<HBAOPlus/GFSDK_SSAO.h>

#include"EffectBase.h"

namespace Gear
{
	namespace Core
	{
		namespace Effect
		{
			class HBAOPlusEffect :public EffectBase
			{
			public:

				static UniquePtr<HBAOPlusEffect> create(GraphicsContext* const context, const uint32_t width, const uint32_t height);

				HBAOPlusEffect(GraphicsContext* const context, const uint32_t width, const uint32_t height);

				~HBAOPlusEffect();

				void imGUICall() override;

				Resource::TextureRenderView* process(Resource::TextureDepthView& depthTexture, Resource::TextureRenderView& gNormal);

			private:

				GFSDK_SSAO_Context_D3D12* aoContext;

				GFSDK_SSAO_Parameters aoParameters;

			};
		}
	}
}

#endif // !_GEAR_CORE_EFFECT_HBAOPLUSEFFECT_H_

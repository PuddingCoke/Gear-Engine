#pragma once

#ifndef _GEAR_EFFECT_HBAOPLUSEFFECT_H_
#define _GEAR_EFFECT_HBAOPLUSEFFECT_H_

#include<HBAOPlus/GFSDK_SSAO.h>

#include"EffectBase.h"

namespace Gear::Effect
{
	CREATESAFETYPE(HBAOPlusEffect);

	class HBAOPlusEffect :public EffectBase
	{
	public:

		static HBAOPlusEffectPtr create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		HBAOPlusEffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		~HBAOPlusEffect();

		void imGuiCall() override;

		RenderTextureView* process(DepthTextureView& depthTexture, RenderTextureView& gNormal);

	private:

		GFSDK_SSAO_Context_D3D12* aoContext;

		GFSDK_SSAO_Parameters aoParameters;

	};
}

#endif // !_GEAR_EFFECT_HBAOPLUSEFFECT_H_

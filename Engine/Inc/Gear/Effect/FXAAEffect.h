#pragma once

#ifndef _GEAR_EFFECT_FXAAEFFECT_H_
#define _GEAR_EFFECT_FXAAEFFECT_H_

#include"EffectBase.h"

namespace Gear::Effect
{
	CREATESAFETYPE(FXAAEffect);

	class FXAAEffect :public EffectBase
	{
	public:

		static FXAAEffectPtr create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		FXAAEffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		~FXAAEffect();

		RenderTextureView* process(RenderTextureView& inputTexture) const;

		void imGuiCall() override;

		void setFXAAQualitySubpix(const float fxaaQualitySubpix);

		void setFXAAQualityEdgeThreshold(const float fxaaQualityEdgeThreshold);

		void setFXAAQualityEdgeThresholdMin(const float fxaaQualityEdgeThresholdMin);

	private:

		RenderTextureViewPtr colorLumaTexture;

		struct FXAAParam
		{
			float fxaaQualityRcpFrame; // unused
			float fxaaQualitySubpix;
			float fxaaQualityEdgeThreshold;
			float fxaaQualityEdgeThresholdMin;
		} fxaaParam;

		ShaderPtr colorToColorLumaPS;

		GraphicsStatePtr colorToColorLumaState;

		ShaderPtr fxaaPS;

		GraphicsStatePtr fxaaState;

	};
}

#endif // !_FXAAEFFECT_H_
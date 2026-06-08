#pragma once

#ifndef _GEAR_CORE_EFFECT_FXAAEFFECT_H_
#define _GEAR_CORE_EFFECT_FXAAEFFECT_H_

#include"EffectBase.h"

namespace Gear::Core::Effect
{
	CREATESAFETYPE(FXAAEffect);

	class FXAAEffect :public EffectBase
	{
	public:

		static FXAAEffectPtr create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		FXAAEffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		~FXAAEffect();

		Resource::RenderTextureView* process(Resource::RenderTextureView& inputTexture) const;

		void imGUICall() override;

		void setFXAAQualitySubpix(const float fxaaQualitySubpix);

		void setFXAAQualityEdgeThreshold(const float fxaaQualityEdgeThreshold);

		void setFXAAQualityEdgeThresholdMin(const float fxaaQualityEdgeThresholdMin);

	private:

		Resource::RenderTextureViewPtr colorLumaTexture;

		struct FXAAParam
		{
			float fxaaQualityRcpFrame; // unused
			float fxaaQualitySubpix;
			float fxaaQualityEdgeThreshold;
			float fxaaQualityEdgeThresholdMin;
		} fxaaParam;

		D3D12Core::ShaderPtr colorToColorLumaPS;

		D3D12Core::PipelineStatePtr colorToColorLumaState;

		D3D12Core::ShaderPtr fxaaPS;

		D3D12Core::PipelineStatePtr fxaaState;

	};
}

#endif // !_FXAAEFFECT_H_
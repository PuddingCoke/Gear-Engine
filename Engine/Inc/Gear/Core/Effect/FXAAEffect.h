#pragma once

#ifndef _GEAR_CORE_EFFECT_FXAAEFFECT_H_
#define _GEAR_CORE_EFFECT_FXAAEFFECT_H_

#include"EffectBase.h"

namespace Gear::Core::Effect
{
	class FXAAEffect :public EffectBase
	{
	public:

		static UniquePtr<FXAAEffect> create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		FXAAEffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		~FXAAEffect();

		Resource::RenderTextureView* process(Resource::RenderTextureView& inputTexture) const;

		void imGUICall() override;

		void setFXAAQualitySubpix(const float fxaaQualitySubpix);

		void setFXAAQualityEdgeThreshold(const float fxaaQualityEdgeThreshold);

		void setFXAAQualityEdgeThresholdMin(const float fxaaQualityEdgeThresholdMin);

	private:

		UniquePtr<Resource::RenderTextureView> colorLumaTexture;

		struct FXAAParam
		{
			float fxaaQualityRcpFrame; // unused
			float fxaaQualitySubpix;
			float fxaaQualityEdgeThreshold;
			float fxaaQualityEdgeThresholdMin;
		} fxaaParam;

		UniquePtr<D3D12Core::Shader> colorToColorLumaPS;

		UniquePtr<D3D12Core::PipelineState> colorToColorLumaState;

		UniquePtr<D3D12Core::Shader> fxaaPS;

		UniquePtr<D3D12Core::PipelineState> fxaaState;

	};
}

#endif // !_FXAAEFFECT_H_
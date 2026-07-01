#pragma once

#ifndef _GEAR_EFFECT_BLOOMEFFECT_H_
#define _GEAR_EFFECT_BLOOMEFFECT_H_

#include<Gear/Resource/SwapTexture.h>

#include<Gear/Utils/Math.h>

#include"EffectBase.h"

namespace Gear::Effect
{
	CREATESAFETYPE(BloomEffect);

	class BloomEffect :public EffectBase
	{
	public:

		static constexpr uint32_t blurSteps = 5;

		static constexpr uint32_t iteration[blurSteps] = { 2,3,4,5,6 };

		static BloomEffectPtr create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, ResourceManager& resManager);

		BloomEffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, ResourceManager& resManager);

		~BloomEffect();

		RenderTextureView* process(RenderTextureView& inputTexture);

		void imGuiCall() override;

		void setThreshold(const float threshold);

		void setIntensity(const float intensity);

		void setSoftThreshold(const float softThreshold);

	private:

		void updateCurve(const uint32_t index);

		ShaderPtr bloomFilter;

		GraphicsStatePtr bloomFilterState;

		ComputeStatePtr bloomHBlurState;

		ComputeStatePtr bloomVBlurState;

		ShaderPtr bloomFinal;

		GraphicsStatePtr bloomFinalState;

		ShaderPtr bloomDownSample;

		GraphicsStatePtr bloomDownSampleState;

		ShaderPtr bloomKarisAverage;

		GraphicsStatePtr bloomKarisAverageState;

		GraphicsStatePtr bloomUpSampleState;

		RenderTextureViewPtr lensDirtTexture;

		SwapTexturePtr swapTexture[blurSteps];

		RenderTextureViewPtr filteredTexture;

		DefaultCBufferPtr blurParamBuffer[blurSteps];

		struct BloomParam
		{
			float exposure;
			float gamma;
			float threshold;
			float intensity;
			float softThreshold;
			float lensDirtIntensity;
		}bloomParam;

		struct BlurParam
		{
			float weight[8];
			float offset[8];
			DirectX::XMFLOAT2 texelSize;
			uint32_t iteration;
			float sigma;
			DirectX::XMFLOAT4 padding[11];
		}blurParam[blurSteps];

	};
}

#endif // !_GEAR_EFFECT_BLOOMEFFECT_H_
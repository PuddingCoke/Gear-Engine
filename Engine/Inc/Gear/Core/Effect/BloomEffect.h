#pragma once

#ifndef _GEAR_CORE_EFFECT_BLOOMEFFECT_H_
#define _GEAR_CORE_EFFECT_BLOOMEFFECT_H_

#include<Gear/Core/Resource/SwapTexture.h>

#include<Gear/Utils/Math.h>

#include"EffectBase.h"

namespace Gear::Core::Effect
{
	CREATESAFETYPE(BloomEffect);

	class BloomEffect :public EffectBase
	{
	public:

		static constexpr uint32_t blurSteps = 5;

		static constexpr uint32_t iteration[blurSteps] = { 2,3,4,5,6 };

		static constexpr DirectX::XMUINT2 workGroupSize = { 60,16 };

		static BloomEffectPtr create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, ResourceManager& resManager);

		BloomEffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, ResourceManager& resManager);

		~BloomEffect();

		Resource::RenderTextureView* process(Resource::RenderTextureView& inputTexture);

		void imGUICall() override;

		void setThreshold(const float threshold);

		void setIntensity(const float intensity);

		void setSoftThreshold(const float softThreshold);

	private:

		void updateCurve(const uint32_t index);

		D3D12Core::ShaderPtr bloomFilter;

		D3D12Core::PipelineStatePtr bloomFilterState;

		D3D12Core::ShaderPtr bloomHBlur;

		D3D12Core::PipelineStatePtr bloomHBlurState;

		D3D12Core::ShaderPtr bloomVBlur;

		D3D12Core::PipelineStatePtr bloomVBlurState;

		D3D12Core::ShaderPtr bloomFinal;

		D3D12Core::PipelineStatePtr bloomFinalState;

		D3D12Core::ShaderPtr bloomDownSample;

		D3D12Core::PipelineStatePtr bloomDownSampleState;

		D3D12Core::ShaderPtr bloomKarisAverage;

		D3D12Core::PipelineStatePtr bloomKarisAverageState;

		D3D12Core::PipelineStatePtr bloomUpSampleState;

		DirectX::XMUINT2 resolutions[blurSteps];

		Resource::RenderTextureViewPtr lensDirtTexture;

		Resource::SwapTexturePtr swapTexture[blurSteps];

		Resource::RenderTextureViewPtr filteredTexture;

		Resource::StaticCBufferPtr blurParamBuffer[blurSteps];

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

#endif // !_GEAR_CORE_EFFECT_BLOOMEFFECT_H_
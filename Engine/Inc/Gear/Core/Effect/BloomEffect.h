#pragma once

#ifndef _GEAR_CORE_EFFECT_BLOOMEFFECT_H_
#define _GEAR_CORE_EFFECT_BLOOMEFFECT_H_

#include<Gear/Core/Resource/SwapTexture.h>

#include<Gear/Utils/Math.h>

#include"EffectBase.h"

namespace Gear::Core::Effect
{
	class BloomEffect :public EffectBase
	{
	public:

		static constexpr uint32_t blurSteps = 5;

		static constexpr uint32_t iteration[blurSteps] = { 2,3,4,5,6 };

		static constexpr DirectX::XMUINT2 workGroupSize = { 60,16 };

		static UniquePtr<BloomEffect> create(GraphicsContext* const context, const uint32_t width, const uint32_t height, ResourceManager* const resManager);

		BloomEffect(GraphicsContext* const context, const uint32_t width, const uint32_t height, ResourceManager* const resManager);

		~BloomEffect();

		Resource::TextureRenderView* process(Resource::TextureRenderView& inputTexture);

		void imGUICall() override;

		void setThreshold(const float threshold);

		void setIntensity(const float intensity);

		void setSoftThreshold(const float softThreshold);

	private:

		void updateCurve(const uint32_t index);

		UniquePtr<D3D12Core::Shader> bloomFilter;

		UniquePtr<D3D12Core::PipelineState> bloomFilterState;

		UniquePtr<D3D12Core::Shader> bloomHBlur;

		UniquePtr<D3D12Core::PipelineState> bloomHBlurState;

		UniquePtr<D3D12Core::Shader> bloomVBlur;

		UniquePtr<D3D12Core::PipelineState> bloomVBlurState;

		UniquePtr<D3D12Core::Shader> bloomFinal;

		UniquePtr<D3D12Core::PipelineState> bloomFinalState;

		UniquePtr<D3D12Core::Shader> bloomDownSample;

		UniquePtr<D3D12Core::PipelineState> bloomDownSampleState;

		UniquePtr<D3D12Core::Shader> bloomKarisAverage;

		UniquePtr<D3D12Core::PipelineState> bloomKarisAverageState;

		UniquePtr<D3D12Core::PipelineState> bloomUpSampleState;

		DirectX::XMUINT2 resolutions[blurSteps];

		UniquePtr<Resource::TextureRenderView> lensDirtTexture;

		UniquePtr<Resource::SwapTexture> swapTexture[blurSteps];

		UniquePtr<Resource::TextureRenderView> filteredTexture;

		UniquePtr<Resource::StaticCBuffer> blurParamBuffer[blurSteps];

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
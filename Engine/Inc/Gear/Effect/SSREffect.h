#pragma once

#ifndef _GEAR_EFFECT_SSREFFECT_H_
#define _GEAR_EFFECT_SSREFFECT_H_

#include"EffectBase.h"

namespace Gear::Effect
{
	CREATESAFETYPE(SSREffect);

	//返回一个包含UV（RG）采样坐标以及能见度（BA）的纹理
	class SSREffect :public EffectBase
	{
	public:

		static SSREffectPtr create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		SSREffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		~SSREffect();

		RenderTextureView* process(
			DepthTextureView& depthTexture,
			RenderTextureView& gPosition,
			RenderTextureView& gNormal);

		void imGUICall() override;

	private:

		RenderTextureViewPtr hiZTexture;

		ShaderPtr hiZProcessPS;

		PipelineStatePtr hiZCopyState;

		PipelineStatePtr hiZCreateState;

		PipelineStatePtr hiZProcessState;

		static constexpr uint32_t hiZMiplvel = 4u;

		static constexpr DXGI_FORMAT hiZTextureFormat = FMT::R32F;

		static constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

	};
}
#endif // !_GEAR_EFFECT_SSREFFECT_H_

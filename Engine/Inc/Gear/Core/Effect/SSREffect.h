#pragma once

#ifndef _GEAR_CORE_EFFECT_SSREFFECT_H_
#define _GEAR_CORE_EFFECT_SSREFFECT_H_

#include"EffectBase.h"

namespace Gear::Core::Effect
{
	CREATESAFETYPE(SSREffect);

	//返回一个包含UV（RG）采样坐标以及能见度（BA）的纹理
	class SSREffect :public EffectBase
	{
	public:

		static SSREffectPtr create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		SSREffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		~SSREffect();

		Resource::RenderTextureView* process(
			Resource::DepthTextureView& depthTexture,
			Resource::RenderTextureView& gPosition,
			Resource::RenderTextureView& gNormal);

		void imGUICall() override;

	private:

		Resource::RenderTextureViewPtr hiZTexture;

		D3D12Core::ShaderPtr hiZCopyCS;

		D3D12Core::ShaderPtr hiZCreateCS;

		D3D12Core::ShaderPtr hiZProcessPS;

		D3D12Core::PipelineStatePtr hiZCopyState;

		D3D12Core::PipelineStatePtr hiZCreateState;

		D3D12Core::PipelineStatePtr hiZProcessState;

		static constexpr uint32_t hiZMiplvel = 4u;

		static constexpr DXGI_FORMAT hiZTextureFormat = FMT::R32F;

		static constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

	};
}
#endif // !_GEAR_CORE_EFFECT_SSREFFECT_H_

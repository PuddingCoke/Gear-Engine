#pragma once

#ifndef _GEAR_CORE_EFFECT_SSREFFECT_H_
#define _GEAR_CORE_EFFECT_SSREFFECT_H_

#include"EffectBase.h"

namespace Gear::Core::Effect
{
	//返回一个包含UV（RG）采样坐标以及能见度（BA）的纹理
	class SSREffect :public EffectBase
	{
	public:

		static UniquePtr<SSREffect> create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		SSREffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height);

		~SSREffect();

		Resource::RenderTextureView* process(
			Resource::DepthTextureView& depthTexture,
			Resource::RenderTextureView& gPosition,
			Resource::RenderTextureView& gNormal);

		void imGUICall() override;

	private:

		UniquePtr<Resource::RenderTextureView> hiZTexture;

		UniquePtr<D3D12Core::Shader> hiZCopyCS;

		UniquePtr<D3D12Core::Shader> hiZCreateCS;

		UniquePtr<D3D12Core::Shader> hiZProcessPS;

		UniquePtr<D3D12Core::PipelineState> hiZCopyState;

		UniquePtr<D3D12Core::PipelineState> hiZCreateState;

		UniquePtr<D3D12Core::PipelineState> hiZProcessState;

		static constexpr uint32_t hiZMiplvel = 4u;

		static constexpr DXGI_FORMAT hiZTextureFormat = FMT::R32F;

		static constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

	};
}
#endif // !_GEAR_CORE_EFFECT_SSREFFECT_H_

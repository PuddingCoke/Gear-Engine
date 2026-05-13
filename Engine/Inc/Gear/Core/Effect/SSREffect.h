#pragma once

#ifndef _GEAR_CORE_EFFECT_SSREFFECT_H_
#define _GEAR_CORE_EFFECT_SSREFFECT_H_

#include"EffectBase.h"

namespace Gear
{
	namespace Core
	{
		namespace Effect
		{
			//返回一个包含UV（RG）采样坐标以及能见度（BA）的纹理
			class SSREffect :public EffectBase
			{
			public:

				SSREffect(GraphicsContext* const context, const uint32_t width, const uint32_t height);

				~SSREffect();

				Resource::TextureRenderView* process(
					Resource::TextureDepthView* const depthTexture,
					Resource::TextureRenderView* const gPosition,
					Resource::TextureRenderView* const gNormal);

				void imGUICall() override;

			private:

				Resource::TextureRenderView* hiZTexture;

				D3D12Core::Shader* hiZCopyCS;

				D3D12Core::Shader* hiZCreateCS;

				D3D12Core::Shader* hiZProcessPS;

				D3D12Core::PipelineState* hiZCopyState;

				D3D12Core::PipelineState* hiZCreateState;

				D3D12Core::PipelineState* hiZProcessState;

				static constexpr uint32_t hiZMiplvel = 4u;

				static constexpr DXGI_FORMAT hiZTextureFormat = FMT::R32F;

				static constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

			};
		}
	}
}
#endif // !_GEAR_CORE_EFFECT_SSREFFECT_H_

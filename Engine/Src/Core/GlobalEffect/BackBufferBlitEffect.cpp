#include<Gear/Core/GlobalEffect/BackBufferBlitEffect.h>

#include<Gear/Core/GlobalEffect/Internal/BackBufferBlitEffectInternal.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Graphics.h>

namespace Gear::Core::GlobalEffect::BackBufferBlitEffect
{
	namespace Internal
	{
		struct BackBufferBlitEffectImpl
		{
			D3D12Core::PipelineStatePtr backBufferBlitState;
		}impl;

		void initialize()
		{
			impl.backBufferBlitState = PipelineStateBuilder()
				.setDefaultFullScreenState()
				.setPS(GlobalShader::getFullScreenPS())
				.setRTVFormats({ Graphics::backBufferFormat })
				.build();

			LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(BackBufferBlitEffect));
		}

		void release()
		{
			impl.backBufferBlitState.reset();
		}
	}

	void process(GraphicsContext& contextRef, Resource::RenderTextureView& inputTexture)
	{
		GraphicsContext* const context = &contextRef;

		context->setPipelineState(*Internal::impl.backBufferBlitState);

		context->setDefRenderTarget();

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		SETCONSTS({
		context->setPSConstants({ inputTexture.getAllSRVIndex() }, co);
			});

		context->draw(3, 1, 0, 0);
	}
}

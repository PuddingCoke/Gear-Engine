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
			UniquePtr<D3D12Core::PipelineState> backBufferBlitState;
		}impl;

		void initialize()
		{
			impl.backBufferBlitState = PipelineStateBuilder()
				.setDefaultFullScreenState()
				.setPS(GlobalShader::getFullScreenPS())
				.setRTVFormats({ Graphics::backBufferFormat })
				.build();

			LOGSUCCESS(L"create", LogColor::brightMagenta, TOWSTRING(BackBufferBlitEffect), LogColor::defaultColor, L"succeeded");
		}

		void release()
		{
			impl.backBufferBlitState.reset();
		}
	}

	void process(GraphicsContext* const context, Resource::RenderTextureView& inputTexture)
	{
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

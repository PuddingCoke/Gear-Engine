#include<Gear/Effect/BackBufferBlitEffect.h>

#include<Gear/Effect/Internal/BackBufferBlitEffectInternal.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Graphics.h>

namespace Gear::Effect::BackBufferBlitEffect
{
	namespace Internal
	{
		class BackBufferBlitEffectImpl
		{
		public:

			BackBufferBlitEffectImpl();

			void process(GraphicsContext& contextRef, RenderTextureView& inputTexture);

		private:

			GraphicsStatePtr backBufferBlitState;

		};

		BackBufferBlitEffectImpl::BackBufferBlitEffectImpl()
		{
			backBufferBlitState = PipelineStateBuilder()
				.setDefaultFullScreenState()
				.setPS(*GlobalShader::getFullScreenPS())
				.build();

			LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(BackBufferBlitEffect));
		}

		void BackBufferBlitEffectImpl::process(GraphicsContext& contextRef, RenderTextureView& inputTexture)
		{
			GraphicsContext* const context = &contextRef;

			context->setPipelineState(*backBufferBlitState);

			context->setDefRenderTarget();

			context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

			context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

			SETCONSTS({
			context->setPSConstants({ inputTexture.getAllSRVIndex() }, co);
				});

			context->drawQuad();
		}

		UniquePtr<BackBufferBlitEffectImpl> impl;

		void initialize()
		{
			impl = makeUnique<BackBufferBlitEffectImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	void process(GraphicsContext& contextRef, RenderTextureView& inputTexture)
	{
		Internal::impl->process(contextRef, inputTexture);
	}
}

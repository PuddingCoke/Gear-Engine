#include <Gear/Effect/BackBufferBlitEffect.h>

#include <Gear/Effect/Internal/BackBufferBlitEffectInternal.h>

#include <Gear/Core/GlobalShader.h>

#include <Gear/Core/Graphics.h>

namespace Gear::Effect::BackBufferBlitEffect
{
	class BackBufferBlitEffectImpl
	{
	public:

		BackBufferBlitEffectImpl();

		void process(GraphicsContext& refContext, RenderTextureView& inputTexture);

	private:

		GraphicsStatePtr backBufferBlitState;
	};

	BackBufferBlitEffectImpl::BackBufferBlitEffectImpl()
	{
		backBufferBlitState = PipelineStateBuilder()
		                          .setDefaultFullScreenState()
		                          .setPS(*GlobalShader::getFullScreenPS())
		                          .build();

		LOGSUCCESS() << "创建" << COLORIZESTRUCT(BackBufferBlitEffect);
	}

	void BackBufferBlitEffectImpl::process(GraphicsContext& refContext, RenderTextureView& inputTexture)
	{
		GraphicsContext* const context = &refContext;

		context->setPipelineState(*backBufferBlitState);

		context->setDefRenderTarget();

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		SETCONSTS({ context->setPSConstants({ inputTexture.getAllSRVIndex() }, co); });

		context->drawQuad();
	}

	UniquePtr<BackBufferBlitEffectImpl> impl;

	namespace Internal
	{
		void initialize()
		{
			impl = makeUnique<BackBufferBlitEffectImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	void process(GraphicsContext& refContext, RenderTextureView& inputTexture)
	{
		impl->process(refContext, inputTexture);
	}
}

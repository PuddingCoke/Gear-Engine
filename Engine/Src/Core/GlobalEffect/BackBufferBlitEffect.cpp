#include<Gear/Core/GlobalEffect/BackBufferBlitEffect.h>

#include<Gear/Core/GlobalEffect/Internal/BackBufferBlitEffectInternal.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Graphics.h>

namespace
{
	struct BackBufferBlitEffectImpl
	{
		UniquePtr<Gear::Core::D3D12Core::PipelineState> backBufferBlitState;
	}impl;
}

void Gear::Core::GlobalEffect::BackBufferBlitEffect::process(GraphicsContext* const context, Resource::TextureRenderView& inputTexture)
{
	context->setPipelineState(*impl.backBufferBlitState);

	context->setDefRenderTarget();

	context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

	context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

	context->setPSConstants({ inputTexture.getAllSRVIndex() }, 0);

	context->draw(3, 1, 0, 0);
}

void Gear::Core::GlobalEffect::BackBufferBlitEffect::Internal::initialize()
{
	impl.backBufferBlitState = PipelineStateBuilder()
		.setDefaultFullScreenState()
		.setPS(GlobalShader::getFullScreenPS())
		.setRTVFormats({ Graphics::backBufferFormat })
		.build();

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"BackBufferBlitEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::BackBufferBlitEffect::Internal::release()
{
	impl.backBufferBlitState.reset();
}

#include<Gear/Core/GlobalEffect/BackBufferBlitEffect.h>

#include<Gear/Core/GlobalEffect/Internal/BackBufferBlitEffectInternal.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Graphics.h>

namespace
{
	struct BackBufferBlitEffectPrivate
	{
		Gear::Core::D3D12Core::PipelineState* backBufferBlitState;
	}pvt;
}

void Gear::Core::GlobalEffect::BackBufferBlitEffect::process(GraphicsContext* const context, Resource::TextureRenderView* const inputTexture)
{
	context->setPipelineState(pvt.backBufferBlitState);

	context->setDefRenderTarget();

	context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

	context->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->setPSConstants({ inputTexture->getAllSRVIndex() }, 0);

	context->draw(3, 1, 0, 0);
}

void Gear::Core::GlobalEffect::BackBufferBlitEffect::Internal::initialize()
{
	pvt.backBufferBlitState = PipelineStateBuilder()
		.setDefaultFullScreenState()
		.setPS(GlobalShader::getFullScreenPS())
		.setRTVFormats({ Graphics::backBufferFormat })
		.build();

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"BackBufferBlitEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::BackBufferBlitEffect::Internal::release()
{
	if (pvt.backBufferBlitState)
	{
		delete pvt.backBufferBlitState;
	}
}

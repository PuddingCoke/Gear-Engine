#include<Gear/Core/GlobalEffect/ToneMapEffect.h>

#include<Gear/Core/GlobalEffect/Internal/ToneMapEffectInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/CompiledShaders/ToneMapCS.h>

namespace
{
	struct ToneMapEffectPrivate
	{
		Gear::Core::D3D12Core::Shader* toneMapCS;

		Gear::Core::D3D12Core::PipelineState* toneMapState;

		Gear::Core::Resource::TextureRenderView* outputTexture;
	} pvt;

	constexpr DXGI_FORMAT outputTextureFormat = Gear::Core::FMT::RGBA16UN;
}

Gear::Core::Resource::TextureRenderView* Gear::Core::GlobalEffect::ToneMapEffect::process(GraphicsContext* const context, Resource::TextureRenderView* const inputTexture)
{
	context->setPipelineState(pvt.toneMapState);

	context->setCSConstants({ inputTexture->getSRVMipIndex(0),pvt.outputTexture->getUAVMipIndex(0) }, 0);

	const float exposure = Graphics::getExposure();

	context->setCSConstants(1, &exposure, 2);

	context->dispatch(Graphics::getWidth() / 16 + 1, Graphics::getHeight() / 16 + 1, 1);

	context->uavBarrier({ pvt.outputTexture->getTexture() });

	return pvt.outputTexture;
}

void Gear::Core::GlobalEffect::ToneMapEffect::Internal::initialize()
{
	pvt.toneMapCS = new D3D12Core::Shader(g_ToneMapCSBytes, sizeof(g_ToneMapCSBytes));

	pvt.toneMapState = PipelineStateBuilder::build(pvt.toneMapCS);

	pvt.outputTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true,
		outputTextureFormat, outputTextureFormat, FMT::UNKNOWN);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"ToneMapEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::ToneMapEffect::Internal::release()
{
	if (pvt.outputTexture)
	{
		delete pvt.outputTexture;
	}

	if (pvt.toneMapState)
	{
		delete pvt.toneMapState;
	}

	if (pvt.toneMapCS)
	{
		delete pvt.toneMapCS;
	}
}
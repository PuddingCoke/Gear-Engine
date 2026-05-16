#include<Gear/Core/GlobalEffect/GammaCorrectEffect.h>

#include<Gear/Core/GlobalEffect/Internal/GammaCorrectEffectInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/CompiledShaders/GammaCorrectCS.h>

namespace
{
	struct GammaCorrectEffectPrivate
	{
		Gear::Core::D3D12Core::Shader* gammaCorrectCS;

		Gear::Core::D3D12Core::PipelineState* gammaCorrectState;

		Gear::Core::Resource::TextureRenderView* outputTexture;
	} pvt;

	constexpr DXGI_FORMAT outputTextureFormat = Gear::Core::FMT::RGBA16UN;
}

Gear::Core::Resource::TextureRenderView* Gear::Core::GlobalEffect::GammaCorrectEffect::process(GraphicsContext* const context, Resource::TextureRenderView* const inputTexture)
{
	context->setPipelineState(pvt.gammaCorrectState);

	context->setCSConstants({ inputTexture->getSRVMipIndex(0),pvt.outputTexture->getUAVMipIndex(0) }, 0);

	const float gamma = Graphics::getGamma();

	context->setCSConstants(1, &gamma, 2);

	context->transitionResources();

	context->dispatch(Graphics::getWidth() / 16 + 1, Graphics::getHeight() / 16 + 1, 1);

	context->uavBarrier({ pvt.outputTexture->getTexture() });

	return pvt.outputTexture;
}

void Gear::Core::GlobalEffect::GammaCorrectEffect::Internal::initialize()
{
	pvt.gammaCorrectCS = new D3D12Core::Shader(g_GammaCorrectCSBytes, sizeof(g_GammaCorrectCSBytes));

	pvt.gammaCorrectState = PipelineStateBuilder::build(pvt.gammaCorrectCS);

	pvt.outputTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true,
		outputTextureFormat, outputTextureFormat, FMT::UNKNOWN);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"GammaCorrectEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::GammaCorrectEffect::Internal::release()
{
	if (pvt.outputTexture)
	{
		delete pvt.outputTexture;
	}

	if (pvt.gammaCorrectState)
	{
		delete pvt.gammaCorrectState;
	}

	if (pvt.gammaCorrectCS)
	{
		delete pvt.gammaCorrectCS;
	}
}
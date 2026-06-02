#include<Gear/Core/GlobalEffect/GammaCorrectEffect.h>

#include<Gear/Core/GlobalEffect/Internal/GammaCorrectEffectInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/CompiledShaders/GammaCorrectCS.h>

namespace
{
	struct GammaCorrectEffectImpl
	{
		UniquePtr<Gear::Core::D3D12Core::Shader> gammaCorrectCS;

		UniquePtr<Gear::Core::D3D12Core::PipelineState> gammaCorrectState;

		UniquePtr<Gear::Core::Resource::TextureRenderView> outputTexture;
	} impl;

	constexpr DXGI_FORMAT outputTextureFormat = Gear::Core::FMT::RGBA16UN;
}

Gear::Core::Resource::TextureRenderView* Gear::Core::GlobalEffect::GammaCorrectEffect::process(GraphicsContext* const context, Resource::TextureRenderView& inputTexture)
{
	context->setPipelineState(*impl.gammaCorrectState);

	context->setCSConstants({ inputTexture.getSRVMipIndex(0),impl.outputTexture->getUAVMipIndex(0) }, 0);

	const float gamma = Graphics::getGamma();

	context->setCSConstants(1, &gamma, 2);

	context->dispatch(Graphics::getWidth() / 16 + 1, Graphics::getHeight() / 16 + 1, 1);

	context->uavBarrier({ impl.outputTexture->getTexture() });

	return impl.outputTexture.get();
}

void Gear::Core::GlobalEffect::GammaCorrectEffect::Internal::initialize()
{
	impl.gammaCorrectCS = D3D12Core::Shader::create(g_GammaCorrectCSBytes, sizeof(g_GammaCorrectCSBytes));

	impl.gammaCorrectState = PipelineStateBuilder::build(*impl.gammaCorrectCS);

	impl.outputTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true,
		outputTextureFormat, outputTextureFormat, FMT::UNKNOWN);

	impl.outputTexture->getTexture()->setName(L"Gamma Corrected Texture");

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"GammaCorrectEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::GammaCorrectEffect::Internal::release()
{
	impl.gammaCorrectCS.reset();

	impl.gammaCorrectState.reset();

	impl.outputTexture.reset();
}
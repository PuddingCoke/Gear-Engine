#include<Gear/Core/GlobalEffect/ToneMapEffect.h>

#include<Gear/Core/GlobalEffect/Internal/ToneMapEffectInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/CompiledShaders/ToneMapCS.h>

namespace
{
	struct ToneMapEffectImpl
	{
		UniquePtr<Gear::Core::D3D12Core::Shader> toneMapCS;

		UniquePtr<Gear::Core::D3D12Core::PipelineState> toneMapState;

		UniquePtr<Gear::Core::Resource::TextureRenderView> outputTexture;
	} impl;

	constexpr DXGI_FORMAT outputTextureFormat = Gear::Core::FMT::RGBA16UN;
}

Gear::Core::Resource::TextureRenderView* Gear::Core::GlobalEffect::ToneMapEffect::process(GraphicsContext* const context, Resource::TextureRenderView& inputTexture)
{
	context->setPipelineState(*impl.toneMapState);

	context->setCSConstants({ inputTexture.getSRVMipIndex(0),impl.outputTexture->getUAVMipIndex(0) }, 0);

	const float exposure = Graphics::getExposure();

	context->setCSConstants(1, &exposure, 2);

	context->dispatch(Graphics::getWidth() / 16 + 1, Graphics::getHeight() / 16 + 1, 1);

	context->uavBarrier({ impl.outputTexture->getTexture() });

	return impl.outputTexture.get();
}

void Gear::Core::GlobalEffect::ToneMapEffect::Internal::initialize()
{
	impl.toneMapCS = D3D12Core::Shader::create(g_ToneMapCSBytes, sizeof(g_ToneMapCSBytes));

	impl.toneMapState = PipelineStateBuilder::build(*impl.toneMapCS);

	impl.outputTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true,
		outputTextureFormat, outputTextureFormat, FMT::UNKNOWN);

	impl.outputTexture->getTexture()->setName(L"Tone Mapped Texture");

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"ToneMapEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::ToneMapEffect::Internal::release()
{
	impl.outputTexture.reset();

	impl.toneMapState.reset();

	impl.toneMapCS.reset();
}
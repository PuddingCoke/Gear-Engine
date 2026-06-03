#include<Gear/Core/GlobalEffect/GammaCorrectEffect.h>

#include<Gear/Core/GlobalEffect/Internal/GammaCorrectEffectInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/CompiledShaders/GammaCorrectCS.h>

namespace Gear::Core::GlobalEffect::GammaCorrectEffect
{
	namespace Internal
	{
		struct GammaCorrectEffectImpl
		{
			UniquePtr<D3D12Core::Shader> gammaCorrectCS;

			UniquePtr<D3D12Core::PipelineState> gammaCorrectState;

			UniquePtr<Resource::TextureRenderView> outputTexture;
		} impl;

		constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

		void initialize()
		{
			impl.gammaCorrectCS = D3D12Core::Shader::create(g_GammaCorrectCSBytes, sizeof(g_GammaCorrectCSBytes));

			impl.gammaCorrectState = PipelineStateBuilder::build(*impl.gammaCorrectCS);

			impl.outputTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true,
				outputTextureFormat, outputTextureFormat, FMT::UNKNOWN);

			impl.outputTexture->getTexture()->setName(L"Gamma Corrected Texture");

			LOGSUCCESS(L"create", LogColor::brightMagenta, L"GammaCorrectEffect", LogColor::defaultColor, L"succeeded");
		}

		void release()
		{
			impl.gammaCorrectCS.reset();

			impl.gammaCorrectState.reset();

			impl.outputTexture.reset();
		}
	}

	Resource::TextureRenderView* process(GraphicsContext* const context, Resource::TextureRenderView& inputTexture)
	{
		context->setPipelineState(*Internal::impl.gammaCorrectState);

		context->setCSConstants({ inputTexture.getSRVMipIndex(0),Internal::impl.outputTexture->getUAVMipIndex(0) }, 0);

		const float gamma = Graphics::getGamma();

		context->setCSConstants(1, &gamma, 2);

		context->dispatch(Graphics::getWidth() / 16 + 1, Graphics::getHeight() / 16 + 1, 1);

		context->uavBarrier({ Internal::impl.outputTexture->getTexture() });

		return Internal::impl.outputTexture.get();
	}
}
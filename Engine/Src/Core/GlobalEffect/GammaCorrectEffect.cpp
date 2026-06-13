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
			D3D12Core::ShaderPtr gammaCorrectCS;

			D3D12Core::PipelineStatePtr gammaCorrectState;

			Resource::RenderTextureViewPtr outputTexture;
		} impl;

		constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

		void initialize()
		{
			impl.gammaCorrectCS = D3D12Core::Shader::create(g_GammaCorrectCSBytes, sizeof(g_GammaCorrectCSBytes));

			impl.gammaCorrectState = PipelineStateBuilder::build(*impl.gammaCorrectCS);

			impl.outputTexture = ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true);

			impl.outputTexture->getTexture()->setName(L"Gamma Corrected Texture");

			LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(GammaCorrectEffect));
		}

		void release()
		{
			impl.gammaCorrectCS.reset();

			impl.gammaCorrectState.reset();

			impl.outputTexture.reset();
		}
	}

	Resource::RenderTextureView* process(GraphicsContext& contextRef, Resource::RenderTextureView& inputTexture)
	{
		GraphicsContext* const context = &contextRef;

		context->setPipelineState(*Internal::impl.gammaCorrectState);

		const float gamma = Graphics::getGamma();

		SETCONSTS({
		context->setCSConstants({ inputTexture.getSRVMipIndex(0),Internal::impl.outputTexture->getUAVMipIndex(0) }, co);

		context->setCSConstants(1, &gamma, co);
			});

		context->dispatchDim(Graphics::getWidth(), Graphics::getHeight(), 1);

		return Internal::impl.outputTexture.get();
	}
}
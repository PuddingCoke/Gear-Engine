#include<Gear/Core/GlobalEffect/ToneMapEffect.h>

#include<Gear/Core/GlobalEffect/Internal/ToneMapEffectInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/CompiledShaders/ToneMapCS.h>

namespace Gear::Core::GlobalEffect::ToneMapEffect
{
	namespace Internal
	{
		struct ToneMapEffectImpl
		{
			UniquePtr<D3D12Core::Shader> toneMapCS;

			UniquePtr<D3D12Core::PipelineState> toneMapState;

			UniquePtr<Resource::TextureRenderView> outputTexture;
		} impl;

		constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

		void initialize()
		{
			impl.toneMapCS = D3D12Core::Shader::create(g_ToneMapCSBytes, sizeof(g_ToneMapCSBytes));

			impl.toneMapState = PipelineStateBuilder::build(*impl.toneMapCS);

			impl.outputTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true,
				outputTextureFormat, outputTextureFormat, FMT::UNKNOWN);

			impl.outputTexture->getTexture()->setName(L"Tone Mapped Texture");

			LOGSUCCESS(L"create", LogColor::brightMagenta, TOWSTRING(ToneMapEffect), LogColor::defaultColor, L"succeeded");
		}

		void release()
		{
			impl.outputTexture.reset();

			impl.toneMapState.reset();

			impl.toneMapCS.reset();
		}
	}

	Resource::TextureRenderView* process(GraphicsContext* const context, Resource::TextureRenderView& inputTexture)
	{
		context->setPipelineState(*Internal::impl.toneMapState);

		const float exposure = Graphics::getExposure();

		SETCONSTS({
		context->setCSConstants({ inputTexture.getSRVMipIndex(0),Internal::impl.outputTexture->getUAVMipIndex(0) }, co);

		context->setCSConstants(1, &exposure, co);
			});

		context->dispatch(Graphics::getWidth() / 16 + 1, Graphics::getHeight() / 16 + 1, 1);

		context->uavBarrier({ Internal::impl.outputTexture->getTexture() });

		return Internal::impl.outputTexture.get();
	}
}
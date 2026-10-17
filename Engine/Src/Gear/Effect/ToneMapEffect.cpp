#include<Gear/Effect/ToneMapEffect.h>

#include<Gear/Effect/Internal/ToneMapEffectInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/CompiledShaders/ToneMapCS.h>

namespace Gear::Effect::ToneMapEffect
{
	class ToneMapEffectImpl
	{
	public:

		ToneMapEffectImpl();

		RenderTextureView* process(GraphicsContext& refContext, RenderTextureView& inputTexture);

	private:

		ComputeStatePtr toneMapState;

		RenderTextureViewPtr outputTexture;

		static constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

	};

	ToneMapEffectImpl::ToneMapEffectImpl()
	{
		toneMapState = PipelineStateBuilder::build(Shader::create(g_ToneMapCSBytes, sizeof(g_ToneMapCSBytes)));

		outputTexture = ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true);

		outputTexture->getTexture()->setName(L"Tone Mapped Texture");

		LOGSUCCESS() << "创建" << COLORIZESTRUCT(ToneMapEffect);
	}

	RenderTextureView* ToneMapEffectImpl::process(GraphicsContext& refContext, RenderTextureView& inputTexture)
	{
		GraphicsContext* const context = &refContext;

		context->setPipelineState(*toneMapState);

		const float exposure = Graphics::getExposure();

		SETCONSTS({
		context->setCSConstants({ inputTexture.getSRVMipIndex(0),outputTexture->getUAVMipIndex(0) }, co);

		context->setCSConstants(1, &exposure, co);
			});

		context->dispatchDim(Graphics::getWidth(), Graphics::getHeight(), 1);

		return outputTexture.get();
	}

	UniquePtr<ToneMapEffectImpl> impl;

	namespace Internal
	{

		void initialize()
		{
			impl = makeUnique<ToneMapEffectImpl>();
		}

		void release()
		{
			impl.reset();
		}

	}

	RenderTextureView* process(GraphicsContext& refContext, RenderTextureView& inputTexture)
	{
		return impl->process(refContext, inputTexture);
	}
}

#include<Gear/Effect/GammaCorrectEffect.h>

#include<Gear/Effect/Internal/GammaCorrectEffectInternal.h>

#include<Gear/Core/Graphics.h>

#include<CompiledShaders/GammaCorrectCS.h>

namespace Gear::Effect::GammaCorrectEffect
{
	class GammaCorrectEffectImpl
	{
	public:

		GammaCorrectEffectImpl();

		RenderTextureView* process(GraphicsContext& refContext, RenderTextureView& inputTexture);

	private:

		ComputeStatePtr gammaCorrectState;

		RenderTextureViewPtr outputTexture;

		static constexpr DXGI_FORMAT outputTextureFormat = FMT::RGBA16UN;

	};

	GammaCorrectEffectImpl::GammaCorrectEffectImpl()
	{
		gammaCorrectState = PipelineStateBuilder::build(Shader::create(g_GammaCorrectCSBytes, sizeof(g_GammaCorrectCSBytes)));

		outputTexture = ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), outputTextureFormat, 1, 1, false, true);

		outputTexture->getTexture()->setName(L"Gamma Corrected Texture");

		LOGSUCCESS() << "创建" << COLORIZESTRUCT(GammaCorrectEffect);
	}

	RenderTextureView* GammaCorrectEffectImpl::process(GraphicsContext& refContext, RenderTextureView& inputTexture)
	{
		GraphicsContext* const context = &refContext;

		context->setPipelineState(*gammaCorrectState);

		const float gamma = Graphics::getGamma();

		SETCONSTS({
		context->setCSConstants({ inputTexture.getSRVMipIndex(0),outputTexture->getUAVMipIndex(0) }, co);

		context->setCSConstants(1, &gamma, co);
			});

		context->dispatchDim(Graphics::getWidth(), Graphics::getHeight(), 1);

		return outputTexture.get();
	}

	UniquePtr<GammaCorrectEffectImpl> impl;

	namespace Internal
	{

		void initialize()
		{
			impl = makeUnique<GammaCorrectEffectImpl>();
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

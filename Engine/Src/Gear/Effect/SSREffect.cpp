#include<Gear/Effect/SSREffect.h>

#include<Shaders/CompiledShaders/HiZCopyCS.h>

#include<Shaders/CompiledShaders/HiZCreateCS.h>

#include<Shaders/CompiledShaders/HiZProcessPS.h>

namespace Gear::Effect
{
	SSREffectPtr SSREffect::create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height)
	{
		return makeUnique<SSREffect>(contextRef, width, height);
	}

	SSREffect::SSREffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height) :
		EffectBase(contextRef, width, height, outputTextureFormat),
		hiZTexture(ResourceManager::createComputeTexture(width, height, hiZTextureFormat, 1, hiZMiplvel, false, true))
	{
		hiZProcessPS = Shader::create(g_HiZProcessPSBytes, sizeof(g_HiZProcessPSBytes));

		hiZCopyState = PipelineStateBuilder::build(Shader::create(g_HiZCopyCSBytes, sizeof(g_HiZCopyCSBytes)));

		hiZCreateState = PipelineStateBuilder::build(Shader::create(g_HiZCreateCSBytes, sizeof(g_HiZCreateCSBytes)));

		hiZProcessState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setVS(*GlobalShader::getFullScreenVS())
			.setPS(*hiZProcessPS)
			.build();

		hiZTexture->getTexture()->setName(L"Hi-Z Accelerate Texture");

		outputTexture->getTexture()->setName(L"Hi-Z Output Texture");
	}

	SSREffect::~SSREffect()
	{
	}

	void SSREffect::imGuiCall()
	{

	}

	RenderTextureView* SSREffect::process(DepthTextureView& depthTexture, RenderTextureView& gPosition, RenderTextureView& gNormal)
	{
		context->setPipelineState(*hiZCopyState);

		SETCONSTS({
		context->setCSConstants({ depthTexture.getDepthMipIndex(0),hiZTexture->getUAVMipIndex(0) }, co);
			});

		context->dispatchDim(hiZTexture->get3Dimension());

		context->setPipelineState(*hiZCreateState);

		for (uint32_t i = 0; i < hiZMiplvel - 1; i++)
		{
			SETCONSTS({
			context->setCSConstants({ hiZTexture->getSRVMipIndex(i),hiZTexture->getUAVMipIndex(i + 1) }, co);
				});


			context->dispatchDim(hiZTexture->get3Dimension(i + 1u));
		}

		context->setPipelineState(*hiZProcessState);

		context->setViewportSimple(width, height);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ outputTexture->getRTVMip(0) });

		int maxLevel = static_cast<int>(hiZTexture->getTexture()->getMipLevels() - 1u);

		SETCONSTS({
		context->setPSConstants({ gPosition.getAllSRVIndex(),gNormal.getAllSRVIndex(),hiZTexture->getAllSRVIndex() }, co);

		context->setPSConstants(1, &maxLevel, co);
			});

		context->drawQuad();

		return outputTexture.get();
	}
}
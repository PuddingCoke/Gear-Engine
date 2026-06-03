#include<Gear/Core/Effect/SSREffect.h>

#include<Gear/CompiledShaders/HiZCopyCS.h>

#include<Gear/CompiledShaders/HiZCreateCS.h>

#include<Gear/CompiledShaders/HiZProcessPS.h>

namespace Gear::Core::Effect
{
	UniquePtr<SSREffect> SSREffect::create(GraphicsContext* const context, const uint32_t width, const uint32_t height)
	{
		return makeUnique<SSREffect>(context, width, height);
	}

	SSREffect::SSREffect(GraphicsContext* const context, const uint32_t width, const uint32_t height) :
		EffectBase(context, width, height, outputTextureFormat),
		hiZTexture(ResourceManager::createTextureRenderView(width, height, hiZTextureFormat, 1, hiZMiplvel, false, true, hiZTextureFormat, hiZTextureFormat, FMT::UNKNOWN))
	{
		hiZCopyCS = D3D12Core::Shader::create(g_HiZCopyCSBytes, sizeof(g_HiZCopyCSBytes));

		hiZCreateCS = D3D12Core::Shader::create(g_HiZCreateCSBytes, sizeof(g_HiZCreateCSBytes));

		hiZProcessPS = D3D12Core::Shader::create(g_HiZProcessPSBytes, sizeof(g_HiZProcessPSBytes));

		hiZCopyState = PipelineStateBuilder::build(*hiZCopyCS);

		hiZCreateState = PipelineStateBuilder::build(*hiZCreateCS);

		hiZProcessState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRTVFormats({ outputTextureFormat })
			.setVS(GlobalShader::getFullScreenVS())
			.setPS(*hiZProcessPS)
			.build();

		hiZTexture->getTexture()->setName(L"Hi-Z Accelerate Texture");

		outputTexture->getTexture()->setName(L"Hi-Z Output Texture");
	}

	SSREffect::~SSREffect()
	{
	}

	void SSREffect::imGUICall()
	{

	}

	Resource::TextureRenderView* SSREffect::process(Resource::TextureDepthView& depthTexture, Resource::TextureRenderView& gPosition, Resource::TextureRenderView& gNormal)
	{
		context->setPipelineState(*hiZCopyState);

		context->setCSConstants({ depthTexture.getDepthMipIndex(0),hiZTexture->getUAVMipIndex(0) }, 0);

		context->dispatch(
			dispatchCeil(hiZTexture->getTexture()->getWidth(), 16u),
			dispatchCeil(hiZTexture->getTexture()->getHeight(), 16u),
			1u);

		context->uavBarrier({ hiZTexture->getTexture() });

		context->setPipelineState(*hiZCreateState);

		for (uint32_t i = 0; i < hiZMiplvel - 1; i++)
		{
			context->setCSConstants({ hiZTexture->getSRVMipIndex(i),hiZTexture->getUAVMipIndex(i + 1) }, 0);

			context->dispatch(
				dispatchCeil(hiZTexture->getTexture()->getWidth() >> (i + 1u), 16u),
				dispatchCeil(hiZTexture->getTexture()->getHeight() >> (i + 1u), 16u),
				1u);

			context->uavBarrier({ hiZTexture->getTexture() });
		}

		context->setPipelineState(*hiZProcessState);

		context->setViewportSimple(width, height);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ outputTexture->getRTVMipHandle(0) });

		context->setPSConstants({ gPosition.getAllSRVIndex(),gNormal.getAllSRVIndex(),hiZTexture->getAllSRVIndex() }, 0);

		int maxLevel = static_cast<int>(hiZTexture->getTexture()->getMipLevels() - 1u);

		context->setPSConstants(1, &maxLevel, 3u);

		context->draw(3, 1, 0, 0);

		return outputTexture.get();
	}
}
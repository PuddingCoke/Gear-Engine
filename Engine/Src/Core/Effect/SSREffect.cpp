#include<Gear/Core/Effect/SSREffect.h>

#include<Gear/CompiledShaders/HiZCopyCS.h>

#include<Gear/CompiledShaders/HiZCreateCS.h>

#include<Gear/CompiledShaders/HiZProcessPS.h>

Gear::Core::Effect::SSREffect::SSREffect(GraphicsContext* const context, const uint32_t width, const uint32_t height) :
	EffectBase(context, width, height, outputTextureFormat),
	hiZTexture(ResourceManager::createTextureRenderView(width, height, hiZTextureFormat, 1, hiZMiplvel, false, true, hiZTextureFormat, hiZTextureFormat, FMT::UNKNOWN))
{
	hiZCopyCS = new D3D12Core::Shader(g_HiZCopyCSBytes, sizeof(g_HiZCopyCSBytes));

	hiZCreateCS = new D3D12Core::Shader(g_HiZCreateCSBytes, sizeof(g_HiZCreateCSBytes));

	hiZProcessPS = new D3D12Core::Shader(g_HiZProcessPSBytes, sizeof(g_HiZProcessPSBytes));

	hiZCopyState = PipelineStateBuilder::build(hiZCopyCS);

	hiZCreateState = PipelineStateBuilder::build(hiZCreateCS);

	hiZProcessState = PipelineStateBuilder()
		.setBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT))
		.setRasterizerState(PipelineStateHelper::rasterCullNone)
		.setDepthStencilState(PipelineStateHelper::depthCompareNone)
		.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
		.setRTVFormats({ outputTextureFormat })
		.setVS(GlobalShader::getFullScreenVS())
		.setPS(hiZProcessPS)
		.build();

	hiZTexture->getTexture()->setName(L"Hi-Z Accelerate Texture");

	outputTexture->getTexture()->setName(L"Hi-Z Output Texture");
}

Gear::Core::Effect::SSREffect::~SSREffect()
{
	if (hiZTexture)
		delete hiZTexture;

	if (hiZCopyState)
		delete hiZCopyState;

	if (hiZCreateState)
		delete hiZCreateState;

	if (hiZProcessState)
		delete hiZProcessState;

	if (hiZCopyCS)
		delete hiZCopyCS;

	if (hiZCreateCS)
		delete hiZCreateCS;

	if (hiZProcessPS)
		delete hiZProcessPS;
}

void Gear::Core::Effect::SSREffect::imGUICall()
{

}

Gear::Core::Resource::TextureRenderView* Gear::Core::Effect::SSREffect::process(Resource::TextureDepthView* const depthTexture, Resource::TextureRenderView* const gPosition, Resource::TextureRenderView* const gNormal)
{
	context->setPipelineState(hiZCopyState);

	context->setCSConstants({ depthTexture->getDepthMipIndex(0),hiZTexture->getUAVMipIndex(0) }, 0);

	context->dispatch(
		dispatchCeil(hiZTexture->getTexture()->getWidth(), 16u),
		dispatchCeil(hiZTexture->getTexture()->getHeight(), 16u),
		1u);

	context->uavBarrier({ hiZTexture->getTexture() });

	context->setPipelineState(hiZCreateState);

	for (uint32_t i = 0; i < hiZMiplvel - 1; i++)
	{
		context->setCSConstants({ hiZTexture->getSRVMipIndex(i),hiZTexture->getUAVMipIndex(i + 1) }, 0);

		context->dispatch(
			dispatchCeil(hiZTexture->getTexture()->getWidth() >> (i + 1u), 16u),
			dispatchCeil(hiZTexture->getTexture()->getHeight() >> (i + 1u), 16u),
			1u);

		context->uavBarrier({ hiZTexture->getTexture() });
	}

	context->setPipelineState(hiZProcessState);

	context->setViewportSimple(width, height);

	context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

	context->setRenderTargets({ outputTexture->getRTVMipHandle(0) });

	context->setPSConstants({ gPosition->getAllSRVIndex(),gNormal->getAllSRVIndex(),hiZTexture->getAllSRVIndex() }, 0);

	int maxLevel = static_cast<int>(hiZTexture->getTexture()->getMipLevels() - 1u);

	context->setPSConstants(1, &maxLevel, 3u);

	context->draw(3, 1, 0, 0);

	return outputTexture;
}

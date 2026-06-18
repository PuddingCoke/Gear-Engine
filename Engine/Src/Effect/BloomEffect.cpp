#include<Gear/Effect/BloomEffect.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Utils/File.h>

#include<Shaders/CompiledShaders/BloomFilterPS.h>

#include<Shaders/CompiledShaders/BloomFinalPS.h>

#include<Shaders/CompiledShaders/BloomVBlurCS.h>

#include<Shaders/CompiledShaders/BloomHBlurCS.h>

#include<Shaders/CompiledShaders/BloomDownSamplePS.h>

#include<Shaders/CompiledShaders/BloomKarisAveragePS.h>

namespace Gear::Effect
{
	BloomEffectPtr BloomEffect::create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, ResourceManager& resManager)
	{
		return makeUnique<BloomEffect>(contextRef, width, height, resManager);
	}

	BloomEffect::BloomEffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, ResourceManager& resManager) :
		EffectBase(contextRef, width, height, FMT::RGBA16F),
		lensDirtTexture(resManager.createRenderTextureView(Utils::File::getRootFolder() + L"bloom_dirt_mask.png", true)),
		filteredTexture(ResourceManager::createGraphicsTexture(width, height, FMT::RGBA16F, 1, 1, false, true))
	{
		filteredTexture->getTexture()->setName(L"Bloom Effect Filtered Texture");

		bloomFilter = Shader::create(g_BloomFilterPSBytes, sizeof(g_BloomFilterPSBytes));
		bloomFinal = Shader::create(g_BloomFinalPSBytes, sizeof(g_BloomFinalPSBytes));
		bloomDownSample = Shader::create(g_BloomDownSamplePSBytes, sizeof(g_BloomDownSamplePSBytes));
		bloomKarisAverage = Shader::create(g_BloomKarisAveragePSBytes, sizeof(g_BloomKarisAveragePSBytes));

		{
			const float sigma[blurSteps] = { 0.44f,0.57f,0.8f,1.32f,3.3f };

			for (uint32_t i = 0; i < blurSteps; i++)
			{
				const DirectX::XMUINT2 resolutions = DirectX::XMUINT2(width >> (i + 1), height >> (i + 1));

				swapTexture[i] = ResourceManager::createSwapTexture(
					[=] {
						return ResourceManager::createRenderTextureView(resolutions.x, resolutions.y, FMT::RGBA16F, 1, 1, false, true);
					}
				);

				blurParam[i].texelSize = DirectX::XMFLOAT2(1.f / resolutions.x, 1.f / resolutions.y);
				blurParam[i].iteration = iteration[i];
				blurParam[i].sigma = sigma[i];

				blurParamBuffer[i] = ResourceManager::createDefaultCBuffer(sizeof(BlurParam), true);

				const std::wstring name = L"Blur Param Buffer ";

				blurParamBuffer[i]->getBuffer()->setName((name + std::to_wstring(i)).c_str());

				updateCurve(i);
			}
		}

		bloomParam.exposure = 1.f;
		bloomParam.gamma = 1.f;
		bloomParam.threshold = 1.f;
		bloomParam.intensity = 1.f;
		bloomParam.softThreshold = 1.f;
		bloomParam.lensDirtIntensity = 1.f;

		auto getDefaultBuilder = []
			{
				return PipelineStateBuilder()
					.setBlendState(PipelineStateHelper::blendReplace)
					.setVS(GlobalShader::getFullScreenVS())
					.setRasterizerState(PipelineStateHelper::rasterCullNone)
					.setDepthStencilState(PipelineStateHelper::depthCompareNone)
					.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
					.setRTVFormats({ FMT::RGBA16F });
			};

		bloomFilterState = getDefaultBuilder().setPS(*bloomFilter).build();

		bloomKarisAverageState = getDefaultBuilder().setPS(*bloomKarisAverage).build();

		bloomDownSampleState = getDefaultBuilder().setPS(*bloomDownSample).build();

		bloomUpSampleState = getDefaultBuilder().setPS(GlobalShader::getFullScreenPS()).setBlendState(PipelineStateHelper::blendAddtive).build();

		bloomFinalState = getDefaultBuilder().setPS(*bloomFinal).build();

		bloomHBlurState = PipelineStateBuilder::build(Shader::create(g_BloomHBlurCSBytes, sizeof(g_BloomHBlurCSBytes)));

		bloomVBlurState = PipelineStateBuilder::build(Shader::create(g_BloomVBlurCSBytes, sizeof(g_BloomVBlurCSBytes)));
	}

	BloomEffect::~BloomEffect()
	{
	}

	RenderTextureView* BloomEffect::process(RenderTextureView& inputTexture)
	{
		bloomParam.exposure = Graphics::getExposure();

		bloomParam.gamma = Graphics::getGamma();

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setViewportSimple(width, height);
		context->setPipelineState(*bloomFilterState);
		context->setRenderTargets({ filteredTexture->getRTVMipHandle(0) }, {});
		SETCONSTS({
		context->setPSConstants({ inputTexture.getAllSRVIndex() }, co);
		context->setPSConstants(5, &bloomParam, co);
			});
		context->drawQuad();

		context->setViewportSimple(swapTexture[0]->write()->get2Dimension());
		context->setPipelineState(*bloomKarisAverageState);
		context->setRenderTargets({ swapTexture[0]->write()->getRTVMipHandle(0) }, {});
		SETCONSTS({
		context->setPSConstants({ filteredTexture->getAllSRVIndex() }, co);
		context->setPSConstants(4, &bloomParam, co);
			});
		context->drawQuad();
		swapTexture[0]->swap();

		context->setPipelineState(*bloomDownSampleState);

		for (uint32_t i = 0; i < blurSteps - 1; i++)
		{
			context->setViewportSimple(swapTexture[i + 1]->write()->get2Dimension());
			context->setRenderTargets({ swapTexture[i + 1]->write()->getRTVMipHandle(0) }, {});
			SETCONSTS({
			context->setPSConstants({ swapTexture[i]->read()->getAllSRVIndex() }, co);
				});
			context->drawQuad();
			swapTexture[i + 1]->swap();
		}

		context->setPipelineState(*bloomHBlurState);

		SETCONSTS({
		context->setCSConstants({
			swapTexture[blurSteps - 1]->read()->getAllSRVIndex(),
			swapTexture[blurSteps - 1]->write()->getUAVMipIndex(0) }, co);
			});

		context->setCSConstantBuffer(*blurParamBuffer[blurSteps - 1]);

		context->dispatchDim(swapTexture[blurSteps - 1]->write()->get3Dimension());
		swapTexture[blurSteps - 1]->swap();

		context->setPipelineState(*bloomVBlurState);

		SETCONSTS({
		context->setCSConstants({
			swapTexture[blurSteps - 1]->read()->getAllSRVIndex(),
			swapTexture[blurSteps - 1]->write()->getUAVMipIndex(0) }, co);
			});

		context->setCSConstantBuffer(*blurParamBuffer[blurSteps - 1]);

		context->dispatchDim(swapTexture[blurSteps - 1]->write()->get3Dimension());
		swapTexture[blurSteps - 1]->swap();

		for (uint32_t i = 0; i < blurSteps - 1; i++)
		{
			context->setPipelineState(*bloomHBlurState);

			SETCONSTS({
			context->setCSConstants({
				swapTexture[blurSteps - 2 - i]->read()->getAllSRVIndex(),
				swapTexture[blurSteps - 2 - i]->write()->getUAVMipIndex(0) }, co);
				});

			context->setCSConstantBuffer(*blurParamBuffer[blurSteps - 2 - i]);

			context->dispatchDim(swapTexture[blurSteps - 2 - i]->write()->get3Dimension());
			swapTexture[blurSteps - 2 - i]->swap();

			context->setPipelineState(*bloomVBlurState);

			SETCONSTS({
			context->setCSConstants({
				swapTexture[blurSteps - 2 - i]->read()->getAllSRVIndex(),
				swapTexture[blurSteps - 2 - i]->write()->getUAVMipIndex(0) }, co);
				});

			context->setCSConstantBuffer(*blurParamBuffer[blurSteps - 2 - i]);

			context->dispatchDim(swapTexture[blurSteps - 2 - i]->write()->get3Dimension());

			context->setViewportSimple(swapTexture[blurSteps - 2 - i]->write()->get2Dimension());
			context->setPipelineState(*bloomUpSampleState);
			context->setRenderTargets({ swapTexture[blurSteps - 2 - i]->write()->getRTVMipHandle(0) }, {});
			SETCONSTS({
			context->setPSConstants({ swapTexture[blurSteps - 1 - i]->read()->getAllSRVIndex() }, co);
				});
			context->drawQuad();
			swapTexture[blurSteps - 2 - i]->swap();
		}

		context->setViewportSimple(width, height);
		context->setPipelineState(*bloomFinalState);
		context->setRenderTargets({ outputTexture->getRTVMipHandle(0) }, {});
		SETCONSTS({
		context->setPSConstants({
			inputTexture.getAllSRVIndex(),
			swapTexture[0]->read()->getAllSRVIndex(),
			lensDirtTexture->getAllSRVIndex() }, co);
		context->setPSConstants(6, &bloomParam, co);
			});
		context->drawQuad();

		return outputTexture.get();
	}

	void BloomEffect::imGUICall()
	{
		ImGui::Begin("Bloom Effect");
		ImGui::SliderFloat("Threshold", &bloomParam.threshold, 0.0f, 1.f);
		ImGui::SliderFloat("SoftThreshold", &bloomParam.softThreshold, 0.0f, 1.f);
		ImGui::SliderFloat("Intensity", &bloomParam.intensity, 0.0f, 4.f);
		ImGui::SliderFloat("Lens Dirt Intensity", &bloomParam.lensDirtIntensity, 0.f, 10.f);
		ImGui::End();
	}

	void BloomEffect::setThreshold(const float threshold)
	{
		bloomParam.threshold = threshold;
	}

	void BloomEffect::setIntensity(const float intensity)
	{
		bloomParam.intensity = intensity;
	}

	void BloomEffect::setSoftThreshold(const float softThreshold)
	{
		bloomParam.softThreshold = softThreshold;
	}

	void BloomEffect::updateCurve(const uint32_t index)
	{
		blurParam[index].weight[0] = Utils::Math::gauss(blurParam[index].sigma, 0.f);

		for (uint32_t i = 1; i < (iteration[index] - 1) * 2 + 1; i += 2)
		{
			const float g1 = Utils::Math::gauss(blurParam[index].sigma, (float)i);
			const float g2 = Utils::Math::gauss(blurParam[index].sigma, (float)(i + 1));
			blurParam[index].weight[(i + 1) / 2] = g1 + g2;
			blurParam[index].offset[(i + 1) / 2] = (g1 * i + g2 * (i + 1)) / (g1 + g2);
		}

		context->updateBuffer(*blurParamBuffer[index], &blurParam[index], sizeof(BlurParam));
	}
}

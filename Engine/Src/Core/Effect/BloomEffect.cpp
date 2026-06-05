#include<Gear/Core/Effect/BloomEffect.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Utils/File.h>

#include<Gear/CompiledShaders/BloomFilterPS.h>

#include<Gear/CompiledShaders/BloomFinalPS.h>

#include<Gear/CompiledShaders/BloomVBlurCS.h>

#include<Gear/CompiledShaders/BloomHBlurCS.h>

#include<Gear/CompiledShaders/BloomDownSamplePS.h>

#include<Gear/CompiledShaders/BloomKarisAveragePS.h>

namespace Gear::Core::Effect
{
	UniquePtr<BloomEffect> BloomEffect::create(GraphicsContext* const context, const uint32_t width, const uint32_t height, ResourceManager& resManager)
	{
		return makeUnique<BloomEffect>(context, width, height, resManager);
	}

	BloomEffect::BloomEffect(GraphicsContext* const context, const uint32_t width, const uint32_t height, ResourceManager& resManager) :
		EffectBase(context, width, height, FMT::RGBA16F),
		lensDirtTexture(resManager.createTextureRenderView(Utils::File::getRootFolder() + L"bloom_dirt_mask.png", true)),
		filteredTexture(ResourceManager::createTextureRenderView(width, height, FMT::RGBA16F, 1, 1, false, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F))
	{
		filteredTexture->getTexture()->setName(L"Bloom Effect Filtered Texture");

		bloomFilter = D3D12Core::Shader::create(g_BloomFilterPSBytes, sizeof(g_BloomFilterPSBytes));
		bloomFinal = D3D12Core::Shader::create(g_BloomFinalPSBytes, sizeof(g_BloomFinalPSBytes));
		bloomVBlur = D3D12Core::Shader::create(g_BloomVBlurCSBytes, sizeof(g_BloomVBlurCSBytes));
		bloomHBlur = D3D12Core::Shader::create(g_BloomHBlurCSBytes, sizeof(g_BloomHBlurCSBytes));
		bloomDownSample = D3D12Core::Shader::create(g_BloomDownSamplePSBytes, sizeof(g_BloomDownSamplePSBytes));
		bloomKarisAverage = D3D12Core::Shader::create(g_BloomKarisAveragePSBytes, sizeof(g_BloomKarisAveragePSBytes));

		{
			const float sigma[blurSteps] = { 0.44f,0.57f,0.8f,1.32f,3.3f };

			for (uint32_t i = 0; i < blurSteps; i++)
			{
				resolutions[i] = DirectX::XMUINT2(width >> (i + 1), height >> (i + 1));

				swapTexture[i] = ResourceManager::createSwapTexture(
					[=] {
						return ResourceManager::createTextureRenderView(resolutions[i].x, resolutions[i].y, FMT::RGBA16F, 1, 1, false, true,
							FMT::RGBA16F, FMT::RGBA16F, FMT::RGBA16F);
					}
				);

				blurParam[i].texelSize = DirectX::XMFLOAT2(1.f / resolutions[i].x, 1.f / resolutions[i].y);
				blurParam[i].iteration = iteration[i];
				blurParam[i].sigma = sigma[i];

				blurParamBuffer[i] = ResourceManager::createStaticCBuffer(sizeof(BlurParam), true);

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

		bloomHBlurState = PipelineStateBuilder::build(*bloomHBlur);

		bloomVBlurState = PipelineStateBuilder::build(*bloomVBlur);

	}

	BloomEffect::~BloomEffect()
	{
	}

	Resource::TextureRenderView* BloomEffect::process(Resource::TextureRenderView& inputTexture)
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
		context->draw(3, 1, 0, 0);

		context->setViewportSimple(resolutions[0].x, resolutions[0].y);
		context->setPipelineState(*bloomKarisAverageState);
		context->setRenderTargets({ swapTexture[0]->write()->getRTVMipHandle(0) }, {});
		SETCONSTS({
		context->setPSConstants({ filteredTexture->getAllSRVIndex() }, co);
		context->setPSConstants(4, &bloomParam, co);
			});
		context->draw(3, 1, 0, 0);
		swapTexture[0]->swap();

		context->setPipelineState(*bloomDownSampleState);

		for (uint32_t i = 0; i < blurSteps - 1; i++)
		{
			context->setViewportSimple(resolutions[i + 1].x, resolutions[i + 1].y);
			context->setRenderTargets({ swapTexture[i + 1]->write()->getRTVMipHandle(0) }, {});
			SETCONSTS({
			context->setPSConstants({ swapTexture[i]->read()->getAllSRVIndex() }, co);
				});
			context->draw(3, 1, 0, 0);
			swapTexture[i + 1]->swap();
		}

		context->setPipelineState(*bloomHBlurState);

		SETCONSTS({
		context->setCSConstants({
			swapTexture[blurSteps - 1]->read()->getAllSRVIndex(),
			swapTexture[blurSteps - 1]->write()->getUAVMipIndex(0) }, co);
			});

		context->setCSConstantBuffer(*blurParamBuffer[blurSteps - 1]);

		context->dispatch(resolutions[blurSteps - 1].x / workGroupSize.x, resolutions[blurSteps - 1].y / workGroupSize.y + 1, 1);
		context->uavBarrier({ swapTexture[blurSteps - 1]->write()->getTexture() });
		swapTexture[blurSteps - 1]->swap();

		context->setPipelineState(*bloomVBlurState);

		SETCONSTS({
		context->setCSConstants({
			swapTexture[blurSteps - 1]->read()->getAllSRVIndex(),
			swapTexture[blurSteps - 1]->write()->getUAVMipIndex(0) }, co);
			});

		context->setCSConstantBuffer(*blurParamBuffer[blurSteps - 1]);

		context->dispatch(resolutions[blurSteps - 1].x / workGroupSize.x, resolutions[blurSteps - 1].y / workGroupSize.y + 1, 1);
		context->uavBarrier({ swapTexture[blurSteps - 1]->write()->getTexture() });
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

			context->dispatch(resolutions[blurSteps - 2 - i].x / workGroupSize.x, resolutions[blurSteps - 2 - i].y / workGroupSize.y + 1, 1);
			context->uavBarrier({ swapTexture[blurSteps - 2 - i]->write()->getTexture() });
			swapTexture[blurSteps - 2 - i]->swap();

			context->setPipelineState(*bloomVBlurState);

			SETCONSTS({
			context->setCSConstants({
				swapTexture[blurSteps - 2 - i]->read()->getAllSRVIndex(),
				swapTexture[blurSteps - 2 - i]->write()->getUAVMipIndex(0) }, co);
				});

			context->setCSConstantBuffer(*blurParamBuffer[blurSteps - 2 - i]);

			context->dispatch(resolutions[blurSteps - 2 - i].x / workGroupSize.x, resolutions[blurSteps - 2 - i].y / workGroupSize.y + 1, 1);
			context->uavBarrier({ swapTexture[blurSteps - 2 - i]->write()->getTexture() });

			context->setViewportSimple(resolutions[blurSteps - 2 - i].x, resolutions[blurSteps - 2 - i].y);
			context->setPipelineState(*bloomUpSampleState);
			context->setRenderTargets({ swapTexture[blurSteps - 2 - i]->write()->getRTVMipHandle(0) }, {});
			SETCONSTS({
			context->setPSConstants({ swapTexture[blurSteps - 1 - i]->read()->getAllSRVIndex() }, co);
				});
			context->draw(3, 1, 0, 0);
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
		context->draw(3, 1, 0, 0);

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

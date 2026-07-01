#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		blackHoleShader(Shader::create(File::getRootFolder() + L"BlackHolePS.cso")),
		colorGradingShader(Shader::create(File::getRootFolder() + L"ColorGradingPS.cso")),
		noiseTexture(resManager->createRenderTextureView(L"Noise.png", true)),
		diskTexture(resManager->createRenderTextureView(L"Disk.jpg", true)),
		originTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true)),
		colorGradingTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16UN, 1, 1, false, true)),
		effect(BloomEffect::create(*context, Graphics::getWidth(), Graphics::getHeight(), *resManager))
	{
		pipelineState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setPS(*blackHoleShader)
			.build();

		colorGradingState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setPS(*colorGradingShader)
			.build();

		Keyboard::addKeyDownEvent(Keyboard::K, [this]() {perframeData.useOriginalVer = ~perframeData.useOriginalVer; });

		effect->setIntensity(0.55f);

		Graphics::setExposure(200.f);

		Graphics::setGamma(3.1428571428571428571f);
	}

	~MyRenderTask()
	{
	}

	void imGuiCall() override
	{
		effect->imGuiCall();

		ImGui::Begin("Black Hole Parameters");
		ImGui::Text("Time Elapsed %f", perframeData.timeElapsed);
		ImGui::Checkbox("Use Original Ver", (bool*)&perframeData.useOriginalVer);
		ImGui::SliderFloat("TexturePeriod", &perframeData.texturePeriod, 10.f, 180.f);
		ImGui::SliderFloat("GEXP", &perframeData.GEXP, -1.f, 0.05f);
		ImGui::SliderFloat("Exponent1", &perframeData.exponent1, 0.0f, 2.0f);
		ImGui::SliderFloat("Scale1", &perframeData.scale1, 0.8f, 2.f);
		ImGui::SliderFloat("Bias1", &perframeData.bias1, -0.5f, 2.f);
		ImGui::SliderFloat("Exponent2", &perframeData.exponent2, 0.0f, 2.0f);
		ImGui::SliderFloat("Scale2", &perframeData.scale2, 0, 2.f);
		ImGui::SliderFloat("Bias2", &perframeData.bias2, -0.5f, 0.5f);
		ImGui::SliderFloat("BaseNoise2ScaleFactor", &perframeData.baseNoise2ScaleFactor, 0.f, 1.f);
		ImGui::SliderFloat("Noise2LerpFactor", &perframeData.noise2LerpFactor, 0.f, 1.f);
		ImGui::End();
	}

protected:

	void recordCommand() override
	{
		context->setRenderTargets({ originTexture->getRTVMip(0) });

		context->setPipelineState(*pipelineState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		perframeData.resolution = { static_cast<float>(Graphics::getWidth()) ,static_cast<float>(Graphics::getHeight()) };

		//perframeData.timeElapsed = perframeData.texturePeriod * 3.f + sinf(Graphics::getTimeElapsed()) * 1.f;

		perframeData.timeElapsed = Graphics::getTimeElapsed();

		//perframeData.timeElapsed = 0.f;

		SETCONSTS({
		context->setPSConstants({ noiseTexture->getAllSRVIndex(),diskTexture->getAllSRVIndex() }, co);
		context->setPSConstants(perframeData, co);
			});

		context->drawQuad();

		auto bloomTexture = effect->process(*originTexture);

		auto toneMappedTexture = ToneMapEffect::process(*context, *bloomTexture);

		context->setPipelineState(*colorGradingState);

		context->setRenderTargets({ colorGradingTexture->getRTVMip(0) });

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		SETCONSTS({
		context->setPSConstants({toneMappedTexture->getAllSRVIndex()},co);
			});

		context->drawQuad();

		auto gammaCorrectedTexture = GammaCorrectEffect::process(*context, *colorGradingTexture);

		blit(*gammaCorrectedTexture);
	}

private:

	GraphicsStatePtr pipelineState;

	ShaderPtr blackHoleShader;

	GraphicsStatePtr colorGradingState;

	ShaderPtr colorGradingShader;

	RenderTextureViewPtr noiseTexture;

	RenderTextureViewPtr diskTexture;

	RenderTextureViewPtr originTexture;

	RenderTextureViewPtr colorGradingTexture;

	BloomEffectPtr effect;

	struct PerframeData
	{
		DirectX::XMFLOAT2 resolution;
		float timeElapsed;
		uint32_t useOriginalVer = false;
		float texturePeriod = 120.f;
		float GEXP = 0.03f;
		float exponent1 = 0.6f;
		float scale1 = 1.0f;
		float bias1 = 0.15f;
		float exponent2 = 0.7f;
		float scale2 = 1.2f;
		float bias2 = 0.1f;
		float baseNoise2ScaleFactor = 1.f;
		float noise2LerpFactor = 0.7f;
	} perframeData;

};
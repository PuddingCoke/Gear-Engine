#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

#include"TextBatch.h"

#include"Rain.h"

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		textBatch(makeUnique<TextBatch>(*resManager, L"characters.fnt")),
		originTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA16F, 1, 1, false, true, DirectX::Colors::Black)),
		bloomEffect(BloomEffect::create(*context, Graphics::getWidth(), Graphics::getHeight(), *resManager))
	{
		Rain::stride = static_cast<int>(textBatch->getFontSize());

		for (size_t i = 0; i < Graphics::getWidth() / Rain::stride; i++)
		{
			rains.push_back(Rain((Graphics::getWidth() - Graphics::getWidth() / Rain::stride * Rain::stride) / 2 + i * Rain::stride + Rain::stride / 2.f, Random::genUint() % 6 + 8));
		}

		Graphics::setGamma(1.0f);

		bloomEffect->setThreshold(0.0f);

		bloomEffect->setIntensity(0.5f);
	}

	~MyRenderTask()
	{

	}

	void imGuiCall() override
	{
		bloomEffect->imGuiCall();

		ImGui::Begin("Rain Parameters");
		ImGui::SliderFloat(TOSTRING(colorFactor), &colorFactor, 0.f, 3.f);
		ImGui::SliderFloat(TOSTRING(rainFadePow), &rainFadePow, 0.f, 3.f);
		ImGui::SliderFloat(TOSTRING(goldenFactor), &goldenFactor, 0.f, 1.f);
		ImGui::Checkbox("Logic Running", &logicRunning);
		ImGui::End();
	}

protected:

	void recordCommand() override
	{
		if (logicRunning)
		{
			for (Rain& rain : rains)
			{
				rain.update(Graphics::getDeltaTime());

				if (rain.y + Rain::stride * rain.len < 0)
				{
					rain.reset();
				}
			}
		}

		for (const Rain& rain : rains)
		{
			const float headColor = colorFactor * 1.25f;

			textBatch->drawText(rain.character[0], rain.x, rain.y, 0.f, headColor, headColor, headColor * goldenFactor, 1);

			for (uint32_t j = 1; j < rain.character.size(); j++)
			{
				const float trailColor = colorFactor * std::pow((1.f - static_cast<float>(j) / rain.character.size()), rainFadePow);

				textBatch->drawText(rain.character[j], rain.x, rain.y + Rain::stride * j, 0.f, 0.f, trailColor, 0.f, 1.0f);
			}
		}

		context->clearRenderTarget(originTexture->getRTVMip(0), DirectX::Colors::Black);

		textBatch->render(*context, *originTexture);

		auto bloomTexture = bloomEffect->process(*originTexture);

		auto toneMappedTexture = ToneMapEffect::process(*context, *bloomTexture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(*context, *toneMappedTexture);

		blit(*gammaCorrectedTexture);
	}

private:

	RenderTextureViewPtr originTexture;

	BloomEffectPtr bloomEffect;

	UniquePtr<TextBatch> textBatch;

	std::vector<Rain> rains;

	float colorFactor = 1.0f;

	float rainFadePow = 2.0f;

	float goldenFactor = 0.825f;

	bool logicRunning = true;

};
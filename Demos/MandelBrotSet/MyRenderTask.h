#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		effect(BloomEffect::create(*context, Graphics::getWidth(), Graphics::getHeight(), *resManager)),
		computeCS(Shader::create(File::getRootFolder() + L"ComputeCS.cso")),
		originTexture(ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16UN, 1, 1, false, true))
	{
		computeState = PipelineStateBuilder::build(*computeCS);

		Graphics::setExposure(1.9f);

		Graphics::setGamma(1.2f);

		effect->setIntensity(0.45f);

		param.scale = 0.4f;

		param.lerpFactor = 0.7f;

		param.lerpFactor2 = 1.0f;
	}

	~MyRenderTask()
	{
	}

	void imGUICall() override
	{
		ImGui::Begin("Simulation Parameters");
		ImGui::SliderFloat("scale", &param.scale, 0.f, 1.f);
		ImGui::SliderFloat("lerpFactor", &param.lerpFactor, 0.f, 1.f);
		ImGui::SliderFloat("lerpFactor2", &param.lerpFactor2, 0.f, 1.f);
		ImGui::Text("Position (%f,%f)", param.location.x, param.location.y);
		ImGui::Text("Scale %f", param.scale);
		ImGui::End();

		effect->imGUICall();
	}

protected:

	void recordCommand() override
	{
		if (Input::Mouse::onScroll())
		{
			if (Input::Mouse::getWheelDelta() > 0.f)
			{
				param.scale *= 0.85f;
			}
			else
			{
				param.scale /= 0.85f;
			}

			accParam.frameIndex = 0;
		}

		if (Input::Mouse::getLeftDown() && Input::Mouse::onMove())
		{
			param.location.x -= Graphics::getDeltaTime() * Input::Mouse::getDX() * param.scale;

			param.location.y += Graphics::getDeltaTime() * Input::Mouse::getDY() * param.scale;

			accParam.frameIndex = 0;
		}

		accParam.frameIndex++;

		accParam.floatSeed = Graphics::getTimeElapsed();

		context->setPipelineState(*computeState);

		SETCONSTS({
		context->setCSConstants({ originTexture->getUAVMipIndex(0) }, co);

		context->setCSConstants(param, co);

		context->setCSConstants(accParam, co);
			});

		context->dispatchDim(Graphics::getWidth(), Graphics::getHeight(), 1);

		auto bloomTexture = effect->process(*originTexture);

		auto toneMappedTexture = ToneMapEffect::process(*context, *bloomTexture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(*context, *toneMappedTexture);

		blit(*gammaCorrectedTexture);
	}

private:

	ShaderPtr computeCS;

	PipelineStatePtr computeState;

	RenderTextureViewPtr originTexture;

	struct SimulationParam
	{
		DirectX::XMFLOAT2 location = { 0.f,0.f };
		float scale = { 1.f };
		const DirectX::XMFLOAT2 texelSize = { 1.f / static_cast<float>(Graphics::getWidth()),1.f / static_cast<float>(Graphics::getHeight()) };
		float lerpFactor = 0.f;
		float lerpFactor2 = 0.f;
	} param;

	struct AccumulateParam
	{
		uint32_t frameIndex;
		float floatSeed;
	} accParam;

	BloomEffectPtr effect;

};
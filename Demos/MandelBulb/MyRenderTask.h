#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		accumulateShader(Shader::create(L"AccumulateShader.cso")),
		accumulatedTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, false)),
		accumulateParam{ 0,0.f,8.f },
		POWER(8.f)
	{
		accumulateState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setBlendState(PipelineStateHelper::blendDefault)
			.setPS(*accumulateShader)
			.build();

		Mouse::addMoveEvent([this](const uint32_t)
			{
				if (Mouse::getLeftDown())
				{
					accumulateParam.frameIndex = 0;
				}
			});

		Mouse::addScrollEvent([this](const uint32_t)
			{
				accumulateParam.frameIndex = 0;
			});
	}

	void imGuiCall() override
	{
		ImGui::Begin("Parameters");
		ImGui::SliderFloat("POWER", &POWER, 0.f, 12.f);
		ImGui::End();
	}

	~MyRenderTask()
	{
	}

protected:

	void recordCommand() override
	{
		if (accumulateParam.POWER != POWER)
		{
			accumulateParam.POWER = POWER;

			accumulateParam.frameIndex = 0;
		}

		accumulateParam.frameIndex++;

		accumulateParam.floatSeed = Graphics::getTimeElapsed();

		accumulatedTexture->copyDescriptors();

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPipelineState(*accumulateState);

		SETCONSTS({
		context->setPSConstants(accumulateParam, co);
			});

		context->setRenderTargets({ accumulatedTexture->getRTVMip(0) }, {});

		context->drawQuad();

		blit(*accumulatedTexture);
	}

private:

	ShaderPtr accumulateShader;

	RenderTextureViewPtr accumulatedTexture;

	GraphicsStatePtr accumulateState;

	struct AccumulateParam
	{
		uint32_t frameIndex;
		float floatSeed;
		float POWER;
	} accumulateParam;

	float POWER;

};

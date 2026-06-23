#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		accumulateShader(Shader::create(File::getRootFolder() + L"AccumulateShader.cso")),
		accumulatedTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, false)),
		cameraParam{ Math::pi / 4.f + 0.4f,0.f,3.0f,8.f },
		accumulateParam{ 0,0.f }
	{
		accumulateState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setBlendState(PipelineStateHelper::blendDefault)
			.setRTVFormats({ accumulatedTexture->getRTVFormat() })
			.setPS(*accumulateShader)
			.build();

		Input::Mouse::addMoveEvent([this]()
			{
				if (Input::Mouse::getLeftDown())
				{
					cameraParam.phi -= Input::Mouse::getDY() * Graphics::getDeltaTime();
					cameraParam.theta += Input::Mouse::getDX() * Graphics::getDeltaTime();
					cameraParam.phi = Math::clamp(cameraParam.phi, -Math::halfPi + 0.01f, Math::halfPi - 0.01f);

					accumulateParam.frameIndex = 0;
				}
			});

		Input::Mouse::addScrollEvent([this]()
			{
				cameraParam.radius -= Input::Mouse::getWheelDelta() * 1.f;

				accumulateParam.frameIndex = 0;
			});

	}

	void imGuiCall() override
	{
		ImGui::Begin("Parameters");
		ImGui::SliderFloat("POWER", &cameraParam.POWER, 0.f, 12.f);
		ImGui::End();
	}

	~MyRenderTask()
	{
	}

protected:

	void recordCommand() override
	{
		accumulateParam.frameIndex++;

		accumulateParam.floatSeed = Graphics::getTimeElapsed();

		accumulatedTexture->copyDescriptors();

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPipelineState(*accumulateState);

		SETCONSTS({
		context->setPSConstants(accumulateParam, co);

		context->setPSConstants(cameraParam, co);
			});

		context->setRenderTargets({ accumulatedTexture->getRTVMip(0) }, {});

		context->drawQuad();

		blit(*accumulatedTexture);
	}

private:

	ShaderPtr accumulateShader;

	RenderTextureViewPtr accumulatedTexture;

	PipelineStatePtr accumulateState;

	struct CameraParam
	{
		float phi;
		float theta;
		float radius;
		float POWER;
	} cameraParam;

	struct AccumulateParam
	{
		uint32_t frameIndex;
		float floatSeed;
	} accumulateParam;

};
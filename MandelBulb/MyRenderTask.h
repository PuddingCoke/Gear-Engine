#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		accumulateShader(Shader::create(Utils::File::getRootFolder() + L"AccumulateShader.cso")),
		displayShader(Shader::create(Utils::File::getRootFolder() + L"DisplayShader.cso")),
		accumulatedTexture(ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16UN, 1, 1, false, false,
			FMT::RGBA16UN, FMT::UNKNOWN, FMT::RGBA16UN)),
		cameraParam{ Utils::Math::pi / 4.f + 0.4f,0.f,3.0f,8.f },
		accumulateParam{ 0,0.f }
	{
		accumulateState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setBlendState(PipelineStateHelper::blendDefault)
			.setRTVFormats({ accumulatedTexture->getTexture()->getFormat() })
			.setPS(*accumulateShader)
			.build();

		displayState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setRTVFormats({ Graphics::backBufferFormat })
			.setPS(*displayShader)
			.build();

		Input::Mouse::addMoveEvent([this]()
			{
				if (Input::Mouse::getLeftDown())
				{
					cameraParam.phi -= Input::Mouse::getDY() * Graphics::getDeltaTime();
					cameraParam.theta += Input::Mouse::getDX() * Graphics::getDeltaTime();
					cameraParam.phi = Utils::Math::clamp(cameraParam.phi, -Utils::Math::halfPi + 0.01f, Utils::Math::halfPi - 0.01f);

					accumulateParam.frameIndex = 0;
				}
			});

		Input::Mouse::addScrollEvent([this]()
			{
				cameraParam.radius -= Input::Mouse::getWheelDelta() * 1.f;

				accumulateParam.frameIndex = 0;
			});

	}

	void imGUICall() override
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

		context->setViewport(Graphics::getWidth(), Graphics::getHeight());

		context->setScissorRect(0.f, 0.f, static_cast<float>(Graphics::getWidth()), static_cast<float>(Graphics::getHeight()));

		context->setPipelineState(*accumulateState);

		context->setPSConstants(2, &accumulateParam, 0);

		context->setPSConstants(4, &cameraParam, 2);

		context->setRenderTargets({ accumulatedTexture->getRTVMipHandle(0) }, {});

		context->draw(3, 1, 0, 0);

		context->setPipelineState(*displayState);

		context->setPSConstants({ accumulatedTexture->getAllSRVIndex() }, 0);

		context->setDefRenderTarget();

		context->draw(3, 1, 0, 0);
	}

private:

	UniquePtr<Shader> accumulateShader;

	UniquePtr<Shader> displayShader;

	UniquePtr<TextureRenderView> accumulatedTexture;

	UniquePtr<PipelineState> accumulateState;

	UniquePtr<PipelineState> displayState;

	struct CameraParam
	{
		float phi;
		float theta;
		float radius;
		float POWER;
	} cameraParam;

	struct AccumulateParam
	{
		UINT frameIndex;
		float floatSeed;
	} accumulateParam;

};
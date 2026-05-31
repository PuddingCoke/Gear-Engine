#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask()
	{
		pixelShader = Shader::create(Utils::File::getRootFolder() + L"PixelShader.cso");

		pipelineState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setRTVFormats({ Graphics::backBufferFormat })
			.setPS(*pixelShader)
			.build();
	}

	~MyRenderTask()
	{
	}

protected:

	void recordCommand() override
	{
		context->setDefRenderTarget();

		context->setPipelineState(*pipelineState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->draw(3, 1, 0, 0);
	}

private:

	UniquePtr<PipelineState> pipelineState;

	UniquePtr<Shader> pixelShader;

};
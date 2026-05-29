#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask()
	{
		pixelShader = new Shader(Utils::File::getRootFolder() + L"PixelShader.cso");

		pipelineState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setRTVFormats({ Graphics::backBufferFormat })
			.setPS(pixelShader)
			.build();
	}

	~MyRenderTask()
	{
		delete pipelineState;

		delete pixelShader;
	}

protected:

	void recordCommand() override
	{
		context->setDefRenderTarget();

		context->setPipelineState(pipelineState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->draw(3, 1, 0, 0);
	}

private:

	PipelineState* pipelineState;

	Shader* pixelShader;

};
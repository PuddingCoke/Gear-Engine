#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask()
	{
		pixelShader = Shader::create(File::getWRootFolder() + L"PixelShader.cso");

		pipelineState = PipelineStateBuilder()
			.setDefaultFullScreenState()
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

		context->drawQuad();
	}

private:

	GraphicsStatePtr pipelineState;

	ShaderPtr pixelShader;

};
#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		timer(1.f / 60.f),
		whiteNoiseCS(new Shader(Utils::File::getRootFolder() + L"WhiteNoiseCS.cso")),
		evolveCS(new Shader(Utils::File::getRootFolder() + L"EvolveCS.cso")),
		visualizeCS(new Shader(Utils::File::getRootFolder() + L"VisualizeCS.cso")),
		originTexture(ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, false, true,
			DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN))
	{
		whiteNoiseState = PipelineStateBuilder::build(whiteNoiseCS);

		evolveState = PipelineStateBuilder::build(evolveCS);

		visualizeState = PipelineStateBuilder::build(visualizeCS);

		swapTexture = new SwapTexture([] {return ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), DXGI_FORMAT_R32_FLOAT, 1, 1, false, true,
			DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_UNKNOWN); });

		initialize();
	}

	~MyRenderTask()
	{
		delete whiteNoiseState;

		delete evolveState;
		
		delete visualizeState;

		delete whiteNoiseCS;

		delete evolveCS;

		delete visualizeCS;

		delete swapTexture;

		delete originTexture;
	}

	/*
		float ra = 12.f;
		float b1 = 0.278f;
		float d1 = 0.267f;
		float b2 = 0.365f;
		float d2 = 0.445f;
		float alpha_n = 0.028f;
		float alpha_m = 0.147f;
	*/
	void imGUICall() override
	{
		ImGui::Begin("Simulation Parameters");
		ImGui::SliderFloat("ra", &simulationParam.ra, 0.f, 16.f);
		ImGui::SliderFloat("b1", &simulationParam.b1, 0.f, 1.f);
		ImGui::SliderFloat("d1", &simulationParam.d1, 0.f, 1.f);
		ImGui::SliderFloat("b2", &simulationParam.b2, 0.f, 1.f);
		ImGui::SliderFloat("d2", &simulationParam.d2, 0.f, 1.f);
		ImGui::SliderFloat("alpha_n", &simulationParam.alpha_n, 0.f, 1.f);
		ImGui::SliderFloat("alpha_m", &simulationParam.alpha_m, 0.f, 1.f);
		ImGui::End();
	}

protected:

	void whiteNoise(SwapTexture* swapTexture)
	{
		context->setPipelineState(whiteNoiseState);

		context->setCSConstants({ swapTexture->write()->getUAVMipIndex(0) }, 0);

		UINT uintSeed = Utils::Random::genUint();

		context->setCSConstants(1, &uintSeed, 1);

		context->dispatch(swapTexture->width / 16, swapTexture->height / 9, 1);

		context->uavBarrier({ swapTexture->write()->getTexture() });

		swapTexture->swap();
	}

	void evolve(SwapTexture* swapTexture)
	{
		context->setPipelineState(evolveState);

		context->setCSConstants({ swapTexture->read()->getAllSRVIndex(),
			swapTexture->write()->getUAVMipIndex(0) }, 0);

		context->setCSConstants(8, &simulationParam, 2);

		context->dispatch(swapTexture->width / 16, swapTexture->height / 9, 1);

		context->uavBarrier({ swapTexture->write()->getTexture() });

		swapTexture->swap();
	}

	void initialize()
	{
		whiteNoise(swapTexture);
	}

	void step()
	{
		while (timer.update(Graphics::getDeltaTime()))
		{
			evolve(swapTexture);
		}
	}

	void visualize()
	{
		context->setPipelineState(visualizeState);

		context->setCSConstants({ originTexture->getUAVMipIndex(0),
			swapTexture->read()->getAllSRVIndex() }, 0);

		context->dispatch(swapTexture->width / 16, swapTexture->height / 9, 1);

		context->uavBarrier({ originTexture->getTexture() });

		blit(originTexture);
	}

	void recordCommand() override
	{
		simulationParam.ri = simulationParam.ra / 3.f;

		if (Input::Keyboard::onKeyDown(Input::Keyboard::K))
		{
			whiteNoise(swapTexture);
		}

		step();

		visualize();
	}

private:

	struct SimulationParam
	{
		float ra = 12.f;
		float ri = 4.f;
		float b1 = 0.278f;
		float d1 = 0.267f;
		float b2 = 0.365f;
		float d2 = 0.445f;
		float alpha_n = 0.028f;
		float alpha_m = 0.147f;
	}simulationParam;

	Shader* whiteNoiseCS;

	PipelineState* whiteNoiseState;

	Shader* evolveCS;

	PipelineState* evolveState;

	Shader* visualizeCS;

	PipelineState* visualizeState;

	SwapTexture* swapTexture;

	TextureRenderView* originTexture;

	Utils::Timer timer;

};
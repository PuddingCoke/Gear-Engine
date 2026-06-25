#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		timer(1.f / 60.f),
		originTexture(ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA8UN, 1, 1, false, true))
	{
		whiteNoiseState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"WhiteNoiseCS.cso"));

		evolveState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"EvolveCS.cso"));

		visualizeState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"VisualizeCS.cso"));

		swapTexture = ResourceManager::createSwapTexture([] {return ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::R32F, 1, 1, false, true); });

		initialize();
	}

	~MyRenderTask()
	{
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
	void imGuiCall() override
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

	void whiteNoise(SwapTexture& swapTexture)
	{
		context->setPipelineState(*whiteNoiseState);

		const uint32_t uintSeed = Random::genUint();

		SETCONSTS({
		context->setCSConstants({ swapTexture.write()->getUAVMipIndex(0) }, co);

		context->setCSConstants(1, &uintSeed, co);
			});

		context->dispatchDim(swapTexture.write()->get3Dimension());

		swapTexture.swap();
	}

	void evolve(SwapTexture& swapTexture)
	{
		context->setPipelineState(*evolveState);

		SETCONSTS({
		context->setCSConstants({ swapTexture.read()->getAllSRVIndex(),
			swapTexture.write()->getUAVMipIndex(0) }, co);

		context->setCSConstants(simulationParam, co);
			});

		context->dispatchDim(swapTexture.write()->get3Dimension());

		swapTexture.swap();
	}

	void initialize()
	{
		whiteNoise(*swapTexture);
	}

	void step()
	{
		while (timer.update(Graphics::getDeltaTime()))
		{
			evolve(*swapTexture);
		}
	}

	void visualize()
	{
		context->setPipelineState(*visualizeState);

		SETCONSTS({
		context->setCSConstants({ originTexture->getUAVMipIndex(0),
			swapTexture->read()->getAllSRVIndex() }, co);
			});

		context->dispatchDim(swapTexture->write()->get3Dimension());

		blit(*originTexture);
	}

	void recordCommand() override
	{
		simulationParam.ri = simulationParam.ra / 3.f;

		if (Keyboard::onKeyDown(Keyboard::K))
		{
			whiteNoise(*swapTexture);
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

	ComputeStatePtr whiteNoiseState;

	ComputeStatePtr evolveState;

	ComputeStatePtr visualizeState;

	SwapTexturePtr swapTexture;

	RenderTextureViewPtr originTexture;

	Timer timer;

};
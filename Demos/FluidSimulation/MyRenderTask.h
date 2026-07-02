#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		colorUpdateTimer(1.f)
	{
		const DirectX::XMUINT2 simRes = { Graphics::getWidth() >> config.resolutionFactor,Graphics::getHeight() >> config.resolutionFactor };

		velocityTex = ResourceManager::createSwapTexture([=] {return ResourceManager::createComputeTexture(simRes.x, simRes.y, FMT::RG32F, 1, 1, false, true); });

		velocityTex->setName(L"Velocity Texture");

		colorTex = ResourceManager::createSwapTexture([=] {return ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true); });

		colorTex->setName(L"Color Texture");

		divergenceTex = ResourceManager::createComputeTexture(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true);

		divergenceTex->getTexture()->setName(L"Divergence Texture");

		pressureTex = ResourceManager::createSwapTexture([=] {return ResourceManager::createComputeTexture(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true); });

		pressureTex->setName(L"Pressure Texture");

		vorticityTex = ResourceManager::createComputeTexture(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true);

		vorticityTex->getTexture()->setName(L"Vorticity Texture");

		phongShadeTexture = ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true);

		edgeHighlightTexture = ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA8UN, 1, 1, false, true);

		simulationParamBuffer = ResourceManager::createDynamicCBuffer(sizeof(SimulationParam));

		simulationParam.colorTexelSize = DirectX::XMFLOAT2(1.f / Graphics::getWidth(), 1.f / Graphics::getHeight());

		simulationParam.simTexelSize = DirectX::XMFLOAT2(1.f / simRes.x, 1.f / simRes.y);

		simulationParam.colorTextureSize = DirectX::XMUINT2(Graphics::getWidth(), Graphics::getHeight());

		simulationParam.simTextureSize = DirectX::XMUINT2(simRes.x, simRes.y);

		simulationParam.colorDissipationSpeed = config.colorDissipationSpeed;

		simulationParam.velocityDissipationSpeed = config.velocityDissipationSpeed;

		simulationParam.vorticityIntensity = config.vorticityIntensity;

		splatVelocityState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"SplatVelocityCS.cso"));

		splatColorState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"SplatColorCS.cso"));

		vorticityState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"VorticityCS.cso"));

		vorticityConfinementState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"VorticityConfinementCS.cso"));

		divergenceState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"DivergenceCS.cso"));

		pressureResetState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"PressureResetCS.cso"));

		pressureState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"PressureCS.cso"));

		gradientSubtractState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"GradientSubtractCS.cso"));

		velocityAdvectionState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"VelocityAdvectionCS.cso"));

		colorAdvectionState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"ColorAdvectionCS.cso"));

		velocityBoundaryState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"VelocityBoundaryCS.cso"));

		pressureBoundaryState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"PressureBoundaryCS.cso"));

		phongShadeState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"PhongShadeCS.cso"));

		edgeHighlightState = PipelineStateBuilder::build(Shader::create(File::getWRootFolder() + L"EdgeHighlightCS.cso"));

		effect = BloomEffect::create(*context, Graphics::getWidth(), Graphics::getHeight(), *resManager);

		effect->setThreshold(0.f);

		kDownEventID = Keyboard::addKeyDownEvent(Keyboard::K, [this]()
			{
				config.logicRunning = !config.logicRunning;
			});

		Graphics::setExposure(1.f);

		Graphics::setGamma(1.f);
	}

	~MyRenderTask()
	{
		Keyboard::removeKeyDownEvent(Keyboard::K, kDownEventID);
	}

	void imGuiCall() override
	{
		ImGui::Begin("Parameters");
		ImGui::SliderFloat("color change speed", &config.colorChangeSpeed, 0.f, 50.f);
		ImGui::SliderFloat("color dissipation speed", &simulationParam.colorDissipationSpeed, 0.f, 10.f);
		ImGui::SliderFloat("velocity dissipation speed", &simulationParam.velocityDissipationSpeed, 0.f, 1.f);
		ImGui::SliderFloat("vorticity intensity", &simulationParam.vorticityIntensity, 0.f, 100.f);
		ImGui::SliderFloat("radius", &config.splatRadius, 0.f, 1.0f);
		ImGui::SliderFloat("force", &config.force, 0.f, 10.f);
		ImGui::SliderFloat("kA", &simulationParam.kA, 0.f, 2.f);
		ImGui::SliderFloat("kD", &simulationParam.kD, 0.f, 2.f);
		ImGui::SliderFloat("bumpScale", &config.bumpScale, 100.f, 500.f);
		ImGui::SliderFloat("edgeMagnitudeScale", &simulationParam.edgeMagnitudeScale, 0.f, 6.f);
		ImGui::Checkbox("logic running", &config.logicRunning);
		ImGui::Checkbox("phong shade", &config.phongShading);
		ImGui::Checkbox("edge detection & highlight", &config.edgeHighlight);
		ImGui::Checkbox("vortex", &config.vortex);
		ImGui::End();

		effect->imGuiCall();
	}

	void splatVelocityAndColor()
	{
		if (config.vortex || (Mouse::onMove() && Mouse::getLeftDown()))
		{
			context->setPipelineState(*splatVelocityState);
			SETCONSTS({
			context->setCSConstants({ velocityTex->getAllSRVIndex(),velocityTex->getUAVMipIndex(0) }, co);
				});
			context->dispatchDim(velocityTex->get3Dimension());
			velocityTex->swap();

			context->setPipelineState(*splatColorState);
			SETCONSTS({
			context->setCSConstants({ colorTex->getAllSRVIndex(),colorTex->getUAVMipIndex(0) }, co);
				});
			context->dispatchDim(colorTex->get3Dimension());
			colorTex->swap();
		}
	}

	void vorticityConfinement()
	{
		//calculate vorticity
		context->setPipelineState(*vorticityState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->getAllSRVIndex(),vorticityTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(vorticityTex->get3Dimension());

		//apply vorticity confinement
		context->setPipelineState(*vorticityConfinementState);
		SETCONSTS({
		context->setCSConstants({ vorticityTex->getAllSRVIndex(),velocityTex->getAllSRVIndex(),velocityTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(velocityTex->get3Dimension());
		velocityTex->swap();

		//obstacle
		context->setPipelineState(*velocityBoundaryState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->getAllSRVIndex(),velocityTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(velocityTex->get3Dimension());
		velocityTex->swap();
	}

	void project()
	{
		//calculate divergence
		context->setPipelineState(*divergenceState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->getAllSRVIndex(),divergenceTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(divergenceTex->get3Dimension());

		//reset pressure
		context->setPipelineState(*pressureResetState);
		SETCONSTS({
		context->setCSConstants({ pressureTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(pressureTex->get3Dimension());
		pressureTex->swap();

		//calculate pressure
		for (uint32_t i = 0; i < config.pressureIteraion; i++)
		{
			context->setPipelineState(*pressureState);
			SETCONSTS({
			context->setCSConstants({ divergenceTex->getAllSRVIndex(),pressureTex->getAllSRVIndex(),pressureTex->getUAVMipIndex(0) }, co);
				});
			context->dispatchDim(pressureTex->get3Dimension());
			pressureTex->swap();

			//obstacle
			context->setPipelineState(*pressureBoundaryState);
			SETCONSTS({
			context->setCSConstants({ pressureTex->getAllSRVIndex(),pressureTex->getUAVMipIndex(0) }, co);
				});
			context->dispatchDim(pressureTex->get3Dimension());
			pressureTex->swap();
		}

		//velocity subtract gradient of pressure
		context->setPipelineState(*gradientSubtractState);
		SETCONSTS({
		context->setCSConstants({ pressureTex->getAllSRVIndex(),velocityTex->getAllSRVIndex(),velocityTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(velocityTex->get3Dimension());
		velocityTex->swap();

		//obstacle
		context->setPipelineState(*velocityBoundaryState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->getAllSRVIndex(),velocityTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(velocityTex->get3Dimension());
		velocityTex->swap();
	}

	void advect()
	{
		//velocity advection
		context->setPipelineState(*velocityAdvectionState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->getAllSRVIndex(),velocityTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(velocityTex->get3Dimension());
		velocityTex->swap();

		//obstacle
		context->setPipelineState(*velocityBoundaryState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->getAllSRVIndex(),velocityTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(velocityTex->get3Dimension());
		velocityTex->swap();

		//color advection
		context->setPipelineState(*colorAdvectionState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->getAllSRVIndex(),colorTex->getAllSRVIndex(),colorTex->getUAVMipIndex(0) }, co);
			});
		context->dispatchDim(colorTex->get3Dimension());
		colorTex->swap();
	}

	void recordCommand() override
	{
		const DirectX::XMFLOAT2 pos = { (float)Mouse::getX(),(float)(Graphics::getHeight() - Mouse::getY()) };

		const DirectX::XMFLOAT2 posDelta = { (pos.x - simulationParam.pos.x) * config.force,(pos.y - simulationParam.pos.y) * config.force };

		if (config.vortex)
		{
			simulationParam.pos = DirectX::XMFLOAT2(200.f, Graphics::getHeight() / 2.f);

			simulationParam.splatColor.x *= Graphics::getDeltaTime() * 144.f;

			simulationParam.splatColor.y *= Graphics::getDeltaTime() * 144.f;

			simulationParam.splatColor.z *= Graphics::getDeltaTime() * 144.f;

			simulationParam.posDelta = DirectX::XMFLOAT2(7200.0f * Graphics::getDeltaTime(), 0.0f);
		}
		else
		{
			simulationParam.pos = pos;

			simulationParam.posDelta = posDelta;
		}

		if (colorUpdateTimer.update(Graphics::getDeltaTime() * config.colorChangeSpeed))
		{
			Color c = Color::hsvToRgb({ Random::genFloat(),1.f,1.f });

			simulationParam.splatColor = { c.r,c.g,c.b,1.f };
		}

		simulationParam.splatRadius = config.splatRadius / 50.f;

		simulationParam.bumpScale = 1.f / config.bumpScale;

		simulationParamBuffer->simpleUpdate(&simulationParam);

		context->setGlobalConstantBuffer(*simulationParamBuffer);

		if (config.logicRunning)
		{
			splatVelocityAndColor();

			vorticityConfinement();

			project();

			advect();
		}

		RenderTextureView* outputTexture = nullptr;

		if (config.phongShading)
		{
			context->setPipelineState(*phongShadeState);
			SETCONSTS({
			context->setCSConstants({ colorTex->getAllSRVIndex(),phongShadeTexture->getUAVMipIndex(0) }, co);
				});
			context->dispatchDim(phongShadeTexture->get3Dimension());

			outputTexture = effect->process(*phongShadeTexture);
		}
		else
		{
			outputTexture = effect->process(*colorTex->read());
		}

		RenderTextureView* texture = nullptr;

		if (config.edgeHighlight)
		{
			context->setPipelineState(*edgeHighlightState);
			SETCONSTS({
			context->setCSConstants({ outputTexture->getAllSRVIndex(),edgeHighlightTexture->getUAVMipIndex(0) }, co);
				});
			context->dispatchDim(edgeHighlightTexture->get3Dimension());

			texture = edgeHighlightTexture.get();
		}
		else
		{
			texture = outputTexture;
		}

		auto toneMappedTexture = ToneMapEffect::process(*context, *texture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(*context, *toneMappedTexture);

		blit(*gammaCorrectedTexture);
	}

private:

	BloomEffectPtr effect;

	SwapTexturePtr colorTex;

	SwapTexturePtr velocityTex;

	RenderTextureViewPtr vorticityTex;

	RenderTextureViewPtr divergenceTex;

	SwapTexturePtr pressureTex;

	RenderTextureViewPtr phongShadeTexture;

	RenderTextureViewPtr edgeHighlightTexture;

	struct Configuration
	{
		float colorChangeSpeed = 10.f;//颜色改变速度
		float colorDissipationSpeed = 1.f;//颜色消散速度
		float velocityDissipationSpeed = 0.00f;//速度消散速度
		float vorticityIntensity = 80.f;//涡流强度
		float splatRadius = 0.25f;//施加颜色的半径
		float force = 6.f;//施加速度的力度
		const uint32_t pressureIteraion = 35;//雅可比迭代次数 这个值越高物理模拟越不容易出错 NVIDIA的文章有提到通常20-50次就够了
		const uint32_t resolutionFactor = 1;//物理模拟分辨率 resolution >> resolutionFactor
		float bumpScale = 300.f;
		bool logicRunning = true;
		bool phongShading = true;
		bool edgeHighlight = true;
		bool vortex = false;
	}config;

	struct SimulationParam
	{
		DirectX::XMFLOAT2 pos = { 0,0 };
		DirectX::XMFLOAT2 posDelta = { 0,0 };
		DirectX::XMFLOAT4 splatColor = { 0,0,0,1 };
		DirectX::XMFLOAT2 colorTexelSize;
		DirectX::XMFLOAT2 simTexelSize;
		DirectX::XMUINT2 colorTextureSize;
		DirectX::XMUINT2 simTextureSize;
		float colorDissipationSpeed;
		float velocityDissipationSpeed;
		float vorticityIntensity;
		float splatRadius;
		float kA = 0.6f;
		float kD = 0.4f;
		float bumpScale = 1.f / 300.f;
		float edgeMagnitudeScale = 1.5f;
		DirectX::XMFLOAT4 padding1[10] = {};
	} simulationParam;

	uint64_t kDownEventID;

	DynamicCBufferPtr simulationParamBuffer;

	Timer colorUpdateTimer;

	ComputeStatePtr splatVelocityState;

	ComputeStatePtr splatColorState;

	ComputeStatePtr vorticityState;

	ComputeStatePtr vorticityConfinementState;

	ComputeStatePtr divergenceState;

	ComputeStatePtr pressureResetState;

	ComputeStatePtr pressureState;

	ComputeStatePtr gradientSubtractState;

	ComputeStatePtr velocityAdvectionState;

	ComputeStatePtr colorAdvectionState;

	ComputeStatePtr velocityBoundaryState;

	ComputeStatePtr pressureBoundaryState;

	ComputeStatePtr phongShadeState;

	ComputeStatePtr edgeHighlightState;

};
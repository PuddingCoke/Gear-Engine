#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		colorUpdateTimer(1.f),
		splatVelocityCS(Shader::create(Utils::File::getRootFolder() + L"SplatVelocityCS.cso")),
		splatColorCS(Shader::create(Utils::File::getRootFolder() + L"SplatColorCS.cso")),
		vorticityCS(Shader::create(Utils::File::getRootFolder() + L"VorticityCS.cso")),
		vorticityConfinementCS(Shader::create(Utils::File::getRootFolder() + L"VorticityConfinementCS.cso")),
		divergenceCS(Shader::create(Utils::File::getRootFolder() + L"DivergenceCS.cso")),
		pressureResetCS(Shader::create(Utils::File::getRootFolder() + L"PressureResetCS.cso")),
		pressureCS(Shader::create(Utils::File::getRootFolder() + L"PressureCS.cso")),
		gradientSubtractCS(Shader::create(Utils::File::getRootFolder() + L"GradientSubtractCS.cso")),
		velocityAdvectionCS(Shader::create(Utils::File::getRootFolder() + L"VelocityAdvectionCS.cso")),
		colorAdvectionCS(Shader::create(Utils::File::getRootFolder() + L"ColorAdvectionCS.cso")),
		velocityBoundaryCS(Shader::create(Utils::File::getRootFolder() + L"VelocityBoundaryCS.cso")),
		pressureBoundaryCS(Shader::create(Utils::File::getRootFolder() + L"PressureBoundaryCS.cso")),
		phongShadeCS(Shader::create(Utils::File::getRootFolder() + L"PhongShadeCS.cso")),
		edgeHighlightCS(Shader::create(Utils::File::getRootFolder() + L"EdgeHighlightCS.cso"))
	{
		const DirectX::XMUINT2 simRes = { Graphics::getWidth() >> config.resolutionFactor,Graphics::getHeight() >> config.resolutionFactor };

		velocityTex = ResourceManager::createSwapTexture([=] {return ResourceManager::createComputeTexture(simRes.x, simRes.y, FMT::RG32F, 1, 1, false, true); });

		velocityTex->read()->getTexture()->getResource()->SetName(L"Velocity Texture (0)");

		velocityTex->write()->getTexture()->getResource()->SetName(L"Velocity Texture (1)");

		colorTex = ResourceManager::createSwapTexture([=] {return ResourceManager::createComputeTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true); });

		colorTex->read()->getTexture()->getResource()->SetName(L"Color Texture (0)");

		colorTex->write()->getTexture()->getResource()->SetName(L"Color Texture (1)");

		divergenceTex = ResourceManager::createComputeTexture(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true);

		divergenceTex->getTexture()->getResource()->SetName(L"Divergence Texture");

		pressureTex = ResourceManager::createSwapTexture([=] {return ResourceManager::createComputeTexture(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true); });

		pressureTex->read()->getTexture()->getResource()->SetName(L"Pressure Texture (0)");

		pressureTex->write()->getTexture()->getResource()->SetName(L"Pressure Texture (1)");

		vorticityTex = ResourceManager::createComputeTexture(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true);

		vorticityTex->getTexture()->getResource()->SetName(L"Vorticity Texture");

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

		splatVelocityState = PipelineStateBuilder::build(*splatVelocityCS);

		splatColorState = PipelineStateBuilder::build(*splatColorCS);

		vorticityState = PipelineStateBuilder::build(*vorticityCS);

		vorticityConfinementState = PipelineStateBuilder::build(*vorticityConfinementCS);

		divergenceState = PipelineStateBuilder::build(*divergenceCS);

		pressureResetState = PipelineStateBuilder::build(*pressureResetCS);

		pressureState = PipelineStateBuilder::build(*pressureCS);

		gradientSubtractState = PipelineStateBuilder::build(*gradientSubtractCS);

		velocityAdvectionState = PipelineStateBuilder::build(*velocityAdvectionCS);

		colorAdvectionState = PipelineStateBuilder::build(*colorAdvectionCS);

		velocityBoundaryState = PipelineStateBuilder::build(*velocityBoundaryCS);

		pressureBoundaryState = PipelineStateBuilder::build(*pressureBoundaryCS);

		phongShadeState = PipelineStateBuilder::build(*phongShadeCS);

		edgeHighlightState = PipelineStateBuilder::build(*edgeHighlightCS);

		effect = BloomEffect::create(*context, Graphics::getWidth(), Graphics::getHeight(), *resManager);

		effect->setThreshold(0.f);

		kDownEventID = Input::Keyboard::addKeyDownEvent(Input::Keyboard::K, [this]()
			{
				config.logicRunning = !config.logicRunning;
			});

		Graphics::setExposure(1.f);

		Graphics::setGamma(1.f);
	}

	~MyRenderTask()
	{
		Input::Keyboard::removeKeyDownEvent(Input::Keyboard::K, kDownEventID);
	}

	void imGUICall() override
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

		effect->imGUICall();
	}

	void splatVelocityAndColor()
	{
		if (config.vortex || (Input::Mouse::onMove() && Input::Mouse::getLeftDown()))
		{
			context->setPipelineState(*splatVelocityState);
			SETCONSTS({
			context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, co);
				});
			context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
			velocityTex->swap();

			context->setPipelineState(*splatColorState);
			SETCONSTS({
			context->setCSConstants({ colorTex->read()->getAllSRVIndex(),colorTex->write()->getUAVMipIndex(0) }, co);
				});
			context->dispatch(colorTex->width / 16, colorTex->height / 9, 1);
			colorTex->swap();
		}
	}

	void vorticityConfinement()
	{
		//calculate vorticity
		context->setPipelineState(*vorticityState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),vorticityTex->getUAVMipIndex(0) }, co);
			});
		context->dispatch(vorticityTex->getTexture()->getWidth() / 16, vorticityTex->getTexture()->getHeight() / 9, 1);

		//apply vorticity confinement
		context->setPipelineState(*vorticityConfinementState);
		SETCONSTS({
		context->setCSConstants({ vorticityTex->getAllSRVIndex(),velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, co);
			});
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		velocityTex->swap();

		//obstacle
		context->setPipelineState(*velocityBoundaryState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, co);
			});
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		velocityTex->swap();
	}

	void project()
	{
		//calculate divergence
		context->setPipelineState(*divergenceState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),divergenceTex->getUAVMipIndex(0) }, co);
			});
		context->dispatch(divergenceTex->getTexture()->getWidth() / 16, divergenceTex->getTexture()->getHeight() / 9, 1);

		//reset pressure
		context->setPipelineState(*pressureResetState);
		SETCONSTS({
		context->setCSConstants({ pressureTex->write()->getUAVMipIndex(0) }, co);
			});
		context->dispatch(pressureTex->width / 16, pressureTex->height / 9, 1);
		pressureTex->swap();

		//calculate pressure
		for (uint32_t i = 0; i < config.pressureIteraion; i++)
		{
			context->setPipelineState(*pressureState);
			SETCONSTS({
			context->setCSConstants({ divergenceTex->getAllSRVIndex(),pressureTex->read()->getAllSRVIndex(),pressureTex->write()->getUAVMipIndex(0) }, co);
				});
			context->dispatch(pressureTex->width / 16, pressureTex->height / 9, 1);
			pressureTex->swap();

			//obstacle
			context->setPipelineState(*pressureBoundaryState);
			SETCONSTS({
			context->setCSConstants({ pressureTex->read()->getAllSRVIndex(),pressureTex->write()->getUAVMipIndex(0) }, co);
				});
			context->dispatch(pressureTex->width / 16, pressureTex->height / 9, 1);
			pressureTex->swap();
		}

		//velocity subtract gradient of pressure
		context->setPipelineState(*gradientSubtractState);
		SETCONSTS({
		context->setCSConstants({ pressureTex->read()->getAllSRVIndex(),velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, co);
			});
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		velocityTex->swap();

		//obstacle
		context->setPipelineState(*velocityBoundaryState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, co);
			});
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		velocityTex->swap();
	}

	void advect()
	{
		//velocity advection
		context->setPipelineState(*velocityAdvectionState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, co);
			});
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		velocityTex->swap();

		//obstacle
		context->setPipelineState(*velocityBoundaryState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, co);
			});
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		velocityTex->swap();

		//color advection
		context->setPipelineState(*colorAdvectionState);
		SETCONSTS({
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),colorTex->read()->getAllSRVIndex(),colorTex->write()->getUAVMipIndex(0) }, co);
			});
		context->dispatch(colorTex->width / 16, colorTex->height / 9, 1);
		colorTex->swap();
	}

	void recordCommand() override
	{
		const DirectX::XMFLOAT2 pos = { (float)Input::Mouse::getX(),(float)(Graphics::getHeight() - Input::Mouse::getY()) };

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
			Utils::Color c = Utils::Color::hsvToRgb({ Utils::Random::genFloat(),1.f,1.f });

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
			context->setCSConstants({ colorTex->read()->getAllSRVIndex(),phongShadeTexture->getUAVMipIndex(0) }, co);
				});
			context->dispatch(phongShadeTexture->getTexture()->getWidth() / 16, phongShadeTexture->getTexture()->getHeight() / 9, 1);

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
			context->dispatch(edgeHighlightTexture->getTexture()->getWidth() / 16, edgeHighlightTexture->getTexture()->getHeight() / 9, 1);

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

	Utils::Timer colorUpdateTimer;

	ShaderPtr splatVelocityCS;

	PipelineStatePtr splatVelocityState;

	ShaderPtr splatColorCS;

	PipelineStatePtr splatColorState;

	ShaderPtr vorticityCS;

	PipelineStatePtr vorticityState;

	ShaderPtr vorticityConfinementCS;

	PipelineStatePtr vorticityConfinementState;

	ShaderPtr divergenceCS;

	PipelineStatePtr divergenceState;

	ShaderPtr pressureResetCS;

	PipelineStatePtr pressureResetState;

	ShaderPtr pressureCS;

	PipelineStatePtr pressureState;

	ShaderPtr gradientSubtractCS;

	PipelineStatePtr gradientSubtractState;

	ShaderPtr velocityAdvectionCS;

	PipelineStatePtr velocityAdvectionState;

	ShaderPtr colorAdvectionCS;

	PipelineStatePtr colorAdvectionState;

	ShaderPtr velocityBoundaryCS;

	PipelineStatePtr velocityBoundaryState;

	ShaderPtr pressureBoundaryCS;

	PipelineStatePtr pressureBoundaryState;

	ShaderPtr phongShadeCS;

	PipelineStatePtr phongShadeState;

	ShaderPtr edgeHighlightCS;

	PipelineStatePtr edgeHighlightState;

};
#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/Core/Effect/BloomEffect.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		colorUpdateTimer(1.f),
		splatVelocityCS(new Shader(Utils::File::getRootFolder() + L"SplatVelocityCS.cso")),
		splatColorCS(new Shader(Utils::File::getRootFolder() + L"SplatColorCS.cso")),
		vorticityCS(new Shader(Utils::File::getRootFolder() + L"VorticityCS.cso")),
		vorticityConfinementCS(new Shader(Utils::File::getRootFolder() + L"VorticityConfinementCS.cso")),
		divergenceCS(new Shader(Utils::File::getRootFolder() + L"DivergenceCS.cso")),
		pressureResetCS(new Shader(Utils::File::getRootFolder() + L"PressureResetCS.cso")),
		pressureCS(new Shader(Utils::File::getRootFolder() + L"PressureCS.cso")),
		gradientSubtractCS(new Shader(Utils::File::getRootFolder() + L"GradientSubtractCS.cso")),
		velocityAdvectionCS(new Shader(Utils::File::getRootFolder() + L"VelocityAdvectionCS.cso")),
		colorAdvectionCS(new Shader(Utils::File::getRootFolder() + L"ColorAdvectionCS.cso")),
		velocityBoundaryCS(new Shader(Utils::File::getRootFolder() + L"VelocityBoundaryCS.cso")),
		pressureBoundaryCS(new Shader(Utils::File::getRootFolder() + L"PressureBoundaryCS.cso")),
		phongShadeCS(new Shader(Utils::File::getRootFolder() + L"PhongShadeCS.cso")),
		edgeHighlightCS(new Shader(Utils::File::getRootFolder() + L"EdgeHighlightCS.cso"))
	{
		const DirectX::XMUINT2 simRes = { Graphics::getWidth() >> config.resolutionFactor,Graphics::getHeight() >> config.resolutionFactor };

		velocityTex = new SwapTexture([=] {return ResourceManager::createTextureRenderView(simRes.x, simRes.y, FMT::RG32F, 1, 1, false, true,
			FMT::RG32F, FMT::RG32F, FMT::UNKNOWN); });

		velocityTex->read()->getTexture()->getResource()->SetName(L"Velocity Texture (0)");

		velocityTex->write()->getTexture()->getResource()->SetName(L"Velocity Texture (1)");

		colorTex = new SwapTexture([=] {return ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true,
			FMT::RGBA16F, FMT::RGBA16F, FMT::UNKNOWN); });

		colorTex->read()->getTexture()->getResource()->SetName(L"Color Texture (0)");

		colorTex->write()->getTexture()->getResource()->SetName(L"Color Texture (1)");

		divergenceTex = ResourceManager::createTextureRenderView(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true,
			FMT::R32F, FMT::R32F, FMT::UNKNOWN);

		divergenceTex->getTexture()->getResource()->SetName(L"Divergence Texture");

		pressureTex = new SwapTexture([=] {return ResourceManager::createTextureRenderView(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true,
			FMT::R32F, FMT::R32F, FMT::UNKNOWN); });

		pressureTex->read()->getTexture()->getResource()->SetName(L"Pressure Texture (0)");

		pressureTex->write()->getTexture()->getResource()->SetName(L"Pressure Texture (1)");

		vorticityTex = ResourceManager::createTextureRenderView(simRes.x, simRes.y, FMT::R32F, 1, 1, false, true,
			FMT::R32F, FMT::R32F, FMT::UNKNOWN);

		vorticityTex->getTexture()->getResource()->SetName(L"Vorticity Texture");

		phongShadeTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true,
			FMT::RGBA16F, FMT::RGBA16F, FMT::UNKNOWN);

		edgeHighlightTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA8UN, 1, 1, false, true,
			FMT::RGBA8UN, FMT::RGBA8UN, FMT::UNKNOWN);

		simulationParamBuffer = ResourceManager::createDynamicCBuffer(sizeof(SimulationParam));

		simulationParam.colorTexelSize = DirectX::XMFLOAT2(1.f / Graphics::getWidth(), 1.f / Graphics::getHeight());

		simulationParam.simTexelSize = DirectX::XMFLOAT2(1.f / simRes.x, 1.f / simRes.y);

		simulationParam.colorTextureSize = DirectX::XMUINT2(Graphics::getWidth(), Graphics::getHeight());

		simulationParam.simTextureSize = DirectX::XMUINT2(simRes.x, simRes.y);

		simulationParam.colorDissipationSpeed = config.colorDissipationSpeed;

		simulationParam.velocityDissipationSpeed = config.velocityDissipationSpeed;

		simulationParam.vorticityIntensity = config.vorticityIntensity;

		splatVelocityState = PipelineStateBuilder::build(splatVelocityCS);

		splatColorState = PipelineStateBuilder::build(splatColorCS);

		vorticityState = PipelineStateBuilder::build(vorticityCS);

		vorticityConfinementState = PipelineStateBuilder::build(vorticityConfinementCS);

		divergenceState = PipelineStateBuilder::build(divergenceCS);

		pressureResetState = PipelineStateBuilder::build(pressureResetCS);

		pressureState = PipelineStateBuilder::build(pressureCS);

		gradientSubtractState = PipelineStateBuilder::build(gradientSubtractCS);

		velocityAdvectionState = PipelineStateBuilder::build(velocityAdvectionCS);

		colorAdvectionState = PipelineStateBuilder::build(colorAdvectionCS);

		velocityBoundaryState = PipelineStateBuilder::build(velocityBoundaryCS);

		pressureBoundaryState = PipelineStateBuilder::build(pressureBoundaryCS);

		phongShadeState = PipelineStateBuilder::build(phongShadeCS);

		edgeHighlightState = PipelineStateBuilder::build(edgeHighlightCS);

		effect = new BloomEffect(context, Graphics::getWidth(), Graphics::getHeight(), resManager);

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


		delete effect;

		delete colorTex;
		delete velocityTex;
		delete vorticityTex;
		delete divergenceTex;
		delete pressureTex;

		delete phongShadeTexture;
		delete edgeHighlightTexture;

		delete simulationParamBuffer;

		delete splatVelocityCS;
		delete splatColorCS;
		delete vorticityCS;
		delete vorticityConfinementCS;
		delete divergenceCS;
		delete pressureResetCS;
		delete pressureCS;
		delete gradientSubtractCS;
		delete velocityAdvectionCS;
		delete colorAdvectionCS;
		delete velocityBoundaryCS;
		delete pressureBoundaryCS;
		delete phongShadeCS;
		delete edgeHighlightCS;

		delete splatVelocityState;
		delete splatColorState;
		delete vorticityState;
		delete vorticityConfinementState;
		delete divergenceState;
		delete pressureResetState;
		delete pressureState;
		delete gradientSubtractState;
		delete velocityAdvectionState;
		delete colorAdvectionState;
		delete velocityBoundaryState;
		delete pressureBoundaryState;
		delete phongShadeState;
		delete edgeHighlightState;
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
			context->setPipelineState(splatVelocityState);
			context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, 0);
			context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
			context->uavBarrier({ velocityTex->write()->getTexture() });
			velocityTex->swap();

			context->setPipelineState(splatColorState);
			context->setCSConstants({ colorTex->read()->getAllSRVIndex(),colorTex->write()->getUAVMipIndex(0) }, 0);
			context->dispatch(colorTex->width / 16, colorTex->height / 9, 1);
			context->uavBarrier({ colorTex->write()->getTexture() });
			colorTex->swap();
		}
	}

	void vorticityConfinement()
	{
		//calculate vorticity
		context->setPipelineState(vorticityState);
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),vorticityTex->getUAVMipIndex(0) }, 0);
		context->dispatch(vorticityTex->getTexture()->getWidth() / 16, vorticityTex->getTexture()->getHeight() / 9, 1);
		context->uavBarrier({ vorticityTex->getTexture() });

		//apply vorticity confinement
		context->setPipelineState(vorticityConfinementState);
		context->setCSConstants({ vorticityTex->getAllSRVIndex(),velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, 0);
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		context->uavBarrier({ velocityTex->write()->getTexture() });
		velocityTex->swap();

		//obstacle
		context->setPipelineState(velocityBoundaryState);
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, 0);
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		context->uavBarrier({ velocityTex->write()->getTexture() });
		velocityTex->swap();
	}

	void project()
	{
		//calculate divergence
		context->setPipelineState(divergenceState);
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),divergenceTex->getUAVMipIndex(0) }, 0);
		context->dispatch(divergenceTex->getTexture()->getWidth() / 16, divergenceTex->getTexture()->getHeight() / 9, 1);
		context->uavBarrier({ divergenceTex->getTexture() });

		//reset pressure
		context->setPipelineState(pressureResetState);
		context->setCSConstants({ pressureTex->write()->getUAVMipIndex(0) }, 0);
		context->dispatch(pressureTex->width / 16, pressureTex->height / 9, 1);
		context->uavBarrier({ pressureTex->write()->getTexture() });
		pressureTex->swap();

		//calculate pressure
		for (UINT i = 0; i < config.pressureIteraion; i++)
		{
			context->setPipelineState(pressureState);
			context->setCSConstants({ divergenceTex->getAllSRVIndex(),pressureTex->read()->getAllSRVIndex(),pressureTex->write()->getUAVMipIndex(0) }, 0);
			context->dispatch(pressureTex->width / 16, pressureTex->height / 9, 1);
			context->uavBarrier({ pressureTex->write()->getTexture() });
			pressureTex->swap();

			//obstacle
			context->setPipelineState(pressureBoundaryState);
			context->setCSConstants({ pressureTex->read()->getAllSRVIndex(),pressureTex->write()->getUAVMipIndex(0) }, 0);
			context->dispatch(pressureTex->width / 16, pressureTex->height / 9, 1);
			context->uavBarrier({ pressureTex->write()->getTexture() });
			pressureTex->swap();
		}

		//velocity subtract gradient of pressure
		context->setPipelineState(gradientSubtractState);
		context->setCSConstants({ pressureTex->read()->getAllSRVIndex(),velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, 0);
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		context->uavBarrier({ velocityTex->write()->getTexture() });
		velocityTex->swap();

		//obstacle
		context->setPipelineState(velocityBoundaryState);
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, 0);
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		context->uavBarrier({ velocityTex->write()->getTexture() });
		velocityTex->swap();
	}

	void advect()
	{
		//velocity advection
		context->setPipelineState(velocityAdvectionState);
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, 0);
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		context->uavBarrier({ velocityTex->write()->getTexture() });
		velocityTex->swap();

		//obstacle
		context->setPipelineState(velocityBoundaryState);
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),velocityTex->write()->getUAVMipIndex(0) }, 0);
		context->dispatch(velocityTex->width / 16, velocityTex->height / 9, 1);
		context->uavBarrier({ velocityTex->write()->getTexture() });
		velocityTex->swap();

		//color advection
		context->setPipelineState(colorAdvectionState);
		context->setCSConstants({ velocityTex->read()->getAllSRVIndex(),colorTex->read()->getAllSRVIndex(),colorTex->write()->getUAVMipIndex(0) }, 0);
		context->dispatch(colorTex->width / 16, colorTex->height / 9, 1);
		context->uavBarrier({ colorTex->write()->getTexture() });
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

		context->setGlobalConstantBuffer(simulationParamBuffer);

		if (config.logicRunning)
		{
			splatVelocityAndColor();

			vorticityConfinement();

			project();

			advect();
		}

		TextureRenderView* outputTexture = nullptr;

		if (config.phongShading)
		{
			context->setPipelineState(phongShadeState);
			context->setCSConstants({ colorTex->read()->getAllSRVIndex(),phongShadeTexture->getUAVMipIndex(0) }, 0);
			context->dispatch(phongShadeTexture->getTexture()->getWidth() / 16, phongShadeTexture->getTexture()->getHeight() / 9, 1);
			context->uavBarrier({ phongShadeTexture->getTexture() });

			outputTexture = effect->process(phongShadeTexture);
		}
		else
		{
			outputTexture = effect->process(colorTex->read());
		}

		TextureRenderView* texture = nullptr;

		if (config.edgeHighlight)
		{
			context->setPipelineState(edgeHighlightState);
			context->setCSConstants({ outputTexture->getAllSRVIndex(),edgeHighlightTexture->getUAVMipIndex(0) }, 0);
			context->dispatch(edgeHighlightTexture->getTexture()->getWidth() / 16, edgeHighlightTexture->getTexture()->getHeight() / 9, 1);
			context->uavBarrier({ edgeHighlightTexture->getTexture() });

			texture = edgeHighlightTexture;
		}
		else
		{
			texture = outputTexture;
		}

		auto toneMappedTexture = ToneMapEffect::process(context, texture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(context, toneMappedTexture);

		blit(gammaCorrectedTexture);
	}

private:

	BloomEffect* effect;

	SwapTexture* colorTex;

	SwapTexture* velocityTex;

	TextureRenderView* vorticityTex;

	TextureRenderView* divergenceTex;

	SwapTexture* pressureTex;

	TextureRenderView* phongShadeTexture;

	TextureRenderView* edgeHighlightTexture;

	struct Configuration
	{
		float colorChangeSpeed = 10.f;//颜色改变速度
		float colorDissipationSpeed = 1.f;//颜色消散速度
		float velocityDissipationSpeed = 0.00f;//速度消散速度
		float vorticityIntensity = 80.f;//涡流强度
		float splatRadius = 0.25f;//施加颜色的半径
		float force = 6.f;//施加速度的力度
		const unsigned int pressureIteraion = 35;//雅可比迭代次数 这个值越高物理模拟越不容易出错 NVIDIA的文章有提到通常20-50次就够了
		const unsigned int resolutionFactor = 1;//物理模拟分辨率 resolution >> resolutionFactor
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

	int kDownEventID;

	DynamicCBuffer* simulationParamBuffer;

	Utils::Timer colorUpdateTimer;

	Shader* splatVelocityCS;

	PipelineState* splatVelocityState;

	Shader* splatColorCS;

	PipelineState* splatColorState;

	Shader* vorticityCS;

	PipelineState* vorticityState;

	Shader* vorticityConfinementCS;

	PipelineState* vorticityConfinementState;

	Shader* divergenceCS;

	PipelineState* divergenceState;

	Shader* pressureResetCS;

	PipelineState* pressureResetState;

	Shader* pressureCS;

	PipelineState* pressureState;

	Shader* gradientSubtractCS;

	PipelineState* gradientSubtractState;

	Shader* velocityAdvectionCS;

	PipelineState* velocityAdvectionState;

	Shader* colorAdvectionCS;

	PipelineState* colorAdvectionState;

	Shader* velocityBoundaryCS;

	PipelineState* velocityBoundaryState;

	Shader* pressureBoundaryCS;

	PipelineState* pressureBoundaryState;

	Shader* phongShadeCS;

	PipelineState* phongShadeState;

	Shader* edgeHighlightCS;

	PipelineState* edgeHighlightState;

};
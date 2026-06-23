#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask()
	{
		particleVS = Shader::create(L"ParticleVS.hlsl", ShaderProfile::VERTEX);

		particleGS = Shader::create(L"ParticleGS.hlsl", ShaderProfile::GEOMETRY);

		particlePS = Shader::create(L"ParticlePS.hlsl", ShaderProfile::PIXEL);

		particleComputeState = PipelineStateBuilder::build(Shader::create(L"ParticleCS.hlsl", ShaderProfile::COMPUTE));

		particleRenderState = PipelineStateBuilder()
			.setInputElements({
				InputElementDesc(0),
				InputElementDesc(1)
				})
			.setBlendState(PipelineStateHelper::blendAddtive)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::POINT)
			.setRTVFormats({ FMT::RGBA16F })
			.setDSVFormat(FMT::D32F)
			.setVS(*particleVS)
			.setGS(*particleGS)
			.setPS(*particlePS)
			.build();

		bloomEffect = BloomEffect::create(*context, Graphics::getWidth(), Graphics::getHeight(), *resManager);

		fxaaEffect = FXAAEffect::create(*context, Graphics::getWidth(), Graphics::getHeight());

		originTexture = ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true, DirectX::Colors::Black);

		depthTexture = ResourceManager::createDepthTextureView(Graphics::getWidth(), Graphics::getHeight(), FMT::D32F, 1, 1, false, true);

		{
			UniquePtr<DirectX::XMFLOAT4[]> positions = makeUnique<DirectX::XMFLOAT4[]>(numParticles);

			UniquePtr<DirectX::XMFLOAT4[]> colors = makeUnique<DirectX::XMFLOAT4[]>(numParticles);

			for (uint32_t i = 0; i < numParticles; i++)
			{
				const float radius = 0.3f * Random::genFloat() + 0.3f;

				const float theta = Random::genFloat() * Math::twoPi;

				const float phi = Random::genFloat() * Math::twoPi;

				positions[i] = DirectX::XMFLOAT4(radius * cosf(theta) * sinf(phi), radius * cosf(theta) * cosf(phi), radius * sinf(theta), 1.f);

				if (radius < 0.4f)
				{
					colors[i] = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
				}
				else
				{
					colors[i] = DirectX::XMFLOAT4(1.f, 1.f, radius * 0.4f + 0.1f, 1.f);
				}
			}

			positionBuffer = resManager->createTypedBufferView(FMT::RGBA32F, sizeof(DirectX::XMFLOAT4) * numParticles, false, true, true, false, false, true, positions.get());

			colorBuffer = resManager->createTypedBufferView(FMT::RGBA32F, sizeof(DirectX::XMFLOAT4) * numParticles, false, false, true, false, false, true, colors.get());
		}

		Graphics::setExposure(0.6f);

		Graphics::setGamma(1.f);

		bloomEffect->setThreshold(1.0f);

		simulationParam = { 0.18f,4 };
	}

	~MyRenderTask()
	{
	}

	void imGuiCall() override
	{
		ImGui::Begin("Simulation Param & Particle Param");

		ImGui::SliderFloat("Dissipative Factor", &simulationParam.dissipativeFactor, 0.f, 0.2f);

		ImGui::SliderInt("Simulation Steps", &simulationParam.simulationSteps, 1, 8);

		ImGui::End();

		bloomEffect->imGuiCall();

		fxaaEffect->imGuiCall();
	}

protected:

	void recordCommand() override
	{
		context->setPipelineState(*particleComputeState);

		SETCONSTS({
		context->setCSConstants({ positionBuffer->getUAVIndex() }, co);

		context->setCSConstants(simulationParam, co);
			});

		context->dispatchDim(numParticles, 1, 1);

		context->setPipelineState(*particleRenderState);

		context->setRenderTargets({ originTexture->getRTVMipHandle(0) }, depthTexture->getDSVMipHandle(0));

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::POINTLIST);

		context->setVertexBuffers(0, {
			positionBuffer->getVertexBuffer(),
			colorBuffer->getVertexBuffer()
			});

		SETCONSTS({
		context->setGSConstants(simulationParam, co);
			});

		context->clearRenderTarget(originTexture->getRTVMipHandle(0), DirectX::Colors::Black);

		context->clearDepthStencil(CLEARFLAG::DEPTH, 1.0f, 0);

		context->draw(numParticles, 1, 0, 0);

		auto bloomTexture = bloomEffect->process(*originTexture);

		auto toneMappedTexture = ToneMapEffect::process(*context, *bloomTexture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(*context, *toneMappedTexture);

		blit(*gammaCorrectedTexture);
	}

private:

	constexpr static uint32_t numParticles = 50000;

	struct SimulationParam
	{
		float dissipativeFactor;
		int simulationSteps;
	} simulationParam;

	BloomEffectPtr bloomEffect;

	FXAAEffectPtr fxaaEffect;

	PipelineStatePtr particleComputeState;

	PipelineStatePtr particleRenderState;

	ShaderPtr particleVS;

	ShaderPtr particleGS;

	ShaderPtr particlePS;

	RenderTextureViewPtr originTexture;

	DepthTextureViewPtr depthTexture;

	BufferViewPtr positionBuffer;

	BufferViewPtr colorBuffer;

};
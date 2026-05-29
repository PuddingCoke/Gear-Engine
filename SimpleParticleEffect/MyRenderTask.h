#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/Core/Effect/BloomEffect.h>

#include<Gear/Core/Effect/FXAAEffect.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask()
	{
		particleVS = new Shader(L"ParticleVS.hlsl", DXCCompiler::ShaderProfile::VERTEX);

		particleGS = new Shader(L"ParticleGS.hlsl", DXCCompiler::ShaderProfile::GEOMETRY);

		particlePS = new Shader(L"ParticlePS.hlsl", DXCCompiler::ShaderProfile::PIXEL);

		particleCS = new Shader(L"ParticleCS.hlsl", DXCCompiler::ShaderProfile::COMPUTE);

		particleComputeState = PipelineStateBuilder::build(particleCS);

		particleRenderState = PipelineStateBuilder()
			.setInputElements({
				{"POSITION", 0, FMT::RGBA32F, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"COLOR", 0, FMT::RGBA32F, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
				})
			.setBlendState(PipelineStateHelper::blendAddtive)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::POINT)
			.setRTVFormats({ FMT::RGBA16F })
			.setDSVFormat(FMT::D32F)
			.setVS(particleVS)
			.setGS(particleGS)
			.setPS(particlePS)
			.build();

		bloomEffect = new BloomEffect(context, Core::Graphics::getWidth(), Core::Graphics::getHeight(), resManager);

		fxaaEffect = new FXAAEffect(context, Core::Graphics::getWidth(), Core::Graphics::getHeight());

		originTexture = ResourceManager::createTextureRenderView(Core::Graphics::getWidth(), Core::Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F, DirectX::Colors::Black);

		depthTexture = ResourceManager::createTextureDepthView(Core::Graphics::getWidth(), Core::Graphics::getHeight(), FMT::D32F, 1, 1, false, true);

		{
			DirectX::XMFLOAT4* const positions = new DirectX::XMFLOAT4[numParticles];

			DirectX::XMFLOAT4* const colors = new DirectX::XMFLOAT4[numParticles];

			for (UINT i = 0; i < numParticles; i++)
			{
				const float radius = 0.3f * Utils::Random::genFloat() + 0.3f;

				const float theta = Utils::Random::genFloat() * Utils::Math::twoPi;

				const float phi = Utils::Random::genFloat() * Utils::Math::twoPi;

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

			positionBuffer = resManager->createTypedBufferView(FMT::RGBA32F, sizeof(DirectX::XMFLOAT4) * numParticles, false, true, true, false, false, true, positions);

			colorBuffer = resManager->createTypedBufferView(FMT::RGBA32F, sizeof(DirectX::XMFLOAT4) * numParticles, false, false, true, false, false, true, colors);
		}

		Graphics::setExposure(0.6f);

		Graphics::setGamma(1.f);

		bloomEffect->setThreshold(1.0f);

		simulationParam = { 0.18f,4 };
	}

	~MyRenderTask()
	{
		delete particleComputeState;

		delete particleRenderState;

		delete bloomEffect;
		delete fxaaEffect;

		delete particleVS;
		delete particleGS;
		delete particlePS;

		delete particleCS;

		delete originTexture;

		delete depthTexture;

		delete positionBuffer;
		delete colorBuffer;
	}

	void imGUICall() override
	{
		ImGui::Begin("Simulation Param & Particle Param");

		ImGui::SliderFloat("Dissipative Factor", &simulationParam.dissipativeFactor, 0.f, 0.2f);

		ImGui::SliderInt("Simulation Steps", &simulationParam.simulationSteps, 1, 8);

		ImGui::End();

		bloomEffect->imGUICall();

		fxaaEffect->imGUICall();
	}

protected:

	void recordCommand() override
	{
		context->setPipelineState(particleComputeState);

		context->setCSConstants({ positionBuffer->getUAVIndex() }, 0);

		context->setCSConstants(sizeof(SimulationParam) / 4, &simulationParam, 1);

		context->dispatch(numParticles / 100, 1, 1);

		context->uavBarrier({ positionBuffer->getBuffer() });

		const D3D12Resource::DepthStencilDesc dsDesc = depthTexture->getDSVMipHandle(0);

		context->setPipelineState(particleRenderState);

		context->setRenderTargets({ originTexture->getRTVMipHandle(0) }, dsDesc);

		context->setViewportSimple(Core::Graphics::getWidth(), Core::Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::POINTLIST);

		context->setVertexBuffers(0, {
			positionBuffer->getVertexBuffer(),
			colorBuffer->getVertexBuffer()
			});

		context->setGSConstants(sizeof(SimulationParam) / 4, &simulationParam, 0);

		context->clearRenderTarget(originTexture->getRTVMipHandle(0), DirectX::Colors::Black);

		context->clearDepthStencil(dsDesc, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

		context->draw(numParticles, 1, 0, 0);

		auto bloomTexture = bloomEffect->process(originTexture);

		auto toneMappedTexture = ToneMapEffect::process(context, bloomTexture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(context, toneMappedTexture);

		blit(gammaCorrectedTexture);
	}

private:

	constexpr static UINT numParticles = 50000;

	struct SimulationParam
	{
		float dissipativeFactor;
		int simulationSteps;
	} simulationParam;

	BloomEffect* bloomEffect;

	FXAAEffect* fxaaEffect;

	PipelineState* particleComputeState;

	PipelineState* particleRenderState;

	Shader* particleVS;

	Shader* particleGS;

	Shader* particlePS;

	Shader* particleCS;

	TextureRenderView* originTexture;

	TextureDepthView* depthTexture;

	BufferView* positionBuffer;

	BufferView* colorBuffer;

};
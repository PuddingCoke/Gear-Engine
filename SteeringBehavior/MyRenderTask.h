#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		renderTexture(ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA8UN, 1, 1, false, true, FMT::RGBA8UN, FMT::UNKNOWN, FMT::RGBA8UN, DirectX::Colors::White)),
		arrowTexture(resManager->createTextureRenderView(L"arrow.png", true)),
		stepCS(Shader::create(Utils::File::getRootFolder() + L"StepCS.cso")),
		vehicleVS(Shader::create(Utils::File::getRootFolder() + L"VehicleVS.cso")),
		vehicleGS(Shader::create(Utils::File::getRootFolder() + L"VehicleGS.cso")),
		vehiclePS(Shader::create(Utils::File::getRootFolder() + L"VehiclePS.cso")),
		simulationParam{}
	{
		{
			DirectX::XMFLOAT4* positionVelocityArray = new DirectX::XMFLOAT4[numVehicle];

			DirectX::XMFLOAT2* maxSpeedMaxForceArray = new DirectX::XMFLOAT2[numVehicle];

			for (size_t i = 0; i < numVehicle; i++)
			{
				float angle = Utils::Random::genFloat() * Utils::Math::twoPi;

				float xSpeed = 3.f * cosf(angle);

				float ySpeed = 3.f * sinf(angle);

				positionVelocityArray[i] = DirectX::XMFLOAT4(Utils::Random::genFloat() * Graphics::getWidth(), Utils::Random::genFloat() * Graphics::getHeight(), xSpeed, ySpeed);

				maxSpeedMaxForceArray[i] = DirectX::XMFLOAT2(3.f, 0.1f);
			}

			positionVelocity = ResourceManager::createSwapBuffer(
				[&] {return resManager->createTypedBufferView(FMT::RGBA32F, sizeof(DirectX::XMFLOAT4) * numVehicle, true, true, true, false, false, true, positionVelocityArray); },
				[&] {return ResourceManager::createTypedBufferView(FMT::RGBA32F, sizeof(DirectX::XMFLOAT4) * numVehicle, true, true, true, false, false, true); });

			maxSpeedMaxForce = resManager->createTypedBufferView(FMT::RG32F, sizeof(DirectX::XMFLOAT2) * numVehicle,
				true, false, false, false, false, true, maxSpeedMaxForceArray);

			maxSpeedMaxForce->getBuffer()->setName(L"Max Speed Max Force Buffer");

			delete[] positionVelocityArray;

			delete[] maxSpeedMaxForceArray;
		}

		stepState = PipelineStateBuilder::build(*stepCS);

		vehicleRenderState = PipelineStateBuilder()
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setBlendState(PipelineStateHelper::blendDefault)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setVS(*vehicleVS)
			.setGS(*vehicleGS)
			.setPS(*vehiclePS)
			.setRTVFormats({ renderTexture->getTexture()->getFormat() })
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::POINT)
			.setInputElements(
				{ {"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
				{"VELOCITY",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0} })
			.build();

		simulationParam.numVehicle = numVehicle;

		simulationParam.speedMultiply = 120.f;
	}

	~MyRenderTask()
	{
	}

protected:

	void recordCommand() override
	{
		simulationParam.mousePos = DirectX::XMFLOAT2(Input::Mouse::getX(), Input::Mouse::getY());

		context->setPipelineState(*stepState);

		SETCONSTS({
		context->setCSConstants({
			positionVelocity->write()->getUAVIndex(),
			positionVelocity->read()->getSRVIndex(),
			maxSpeedMaxForce->getSRVIndex() }, co);

		context->setCSConstants(simulationParam, co);
			});

		context->dispatch(dispatchCeil(numVehicle, 32), 1, 1);

		positionVelocity->swap();

		context->setPipelineState(*vehicleRenderState);

		context->setRenderTargets({ renderTexture->getRTVMipHandle(0) });

		context->clearRenderTarget(renderTexture->getRTVMipHandle(0), DirectX::Colors::White);

		context->setViewportSimple(renderTexture->getTexture()->getWidth(), renderTexture->getTexture()->getHeight());

		context->setPrimitiveTopology(TOPOLOGY::POINTLIST);

		context->setVertexBuffers(0, { positionVelocity->read()->getVertexBuffer() });

		SETCONSTS({
		context->setPSConstants({ arrowTexture->getAllSRVIndex() }, co);
			});

		context->draw(numVehicle, 1, 0, 0);

		blit(*renderTexture);
	}

private:

	UniquePtr<SwapBuffer> positionVelocity;

	UniquePtr<BufferView> maxSpeedMaxForce;

	UniquePtr<TextureRenderView> renderTexture;

	UniquePtr<TextureRenderView> arrowTexture;

	struct SimulationParam
	{
		uint32_t numVehicle;
		float speedMultiply;
		DirectX::XMFLOAT2 mousePos;
		float fleeRadius = 75.f;
	} simulationParam;

	static constexpr uint32_t numVehicle = 1000;

	UniquePtr<PipelineState> stepState;

	UniquePtr<Shader> stepCS;

	UniquePtr<PipelineState> vehicleRenderState;

	UniquePtr<Shader> vehicleVS;

	UniquePtr<Shader> vehicleGS;

	UniquePtr<Shader> vehiclePS;

};
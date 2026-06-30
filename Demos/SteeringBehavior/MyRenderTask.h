#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		renderTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA8UN, 1, 1, false, true, DirectX::Colors::White)),
		arrowTexture(resManager->createRenderTextureView(L"arrow.png", true)),
		vehicleVS(Shader::create(File::getRootFolder() + L"VehicleVS.cso")),
		vehicleGS(Shader::create(File::getRootFolder() + L"VehicleGS.cso")),
		vehiclePS(Shader::create(File::getRootFolder() + L"VehiclePS.cso")),
		simulationParam{}
	{
		{
			UniquePtr<DirectX::XMFLOAT4[]> positionVelocityArray = makeUnique<DirectX::XMFLOAT4[]>(numVehicle);

			UniquePtr<DirectX::XMFLOAT2[]> maxSpeedMaxForceArray = makeUnique<DirectX::XMFLOAT2[]>(numVehicle);

			for (size_t i = 0; i < numVehicle; i++)
			{
				float angle = Random::genFloat() * Math::twoPi;

				float xSpeed = 3.f * cosf(angle);

				float ySpeed = 3.f * sinf(angle);

				positionVelocityArray[i] = DirectX::XMFLOAT4(Random::genFloat() * Graphics::getWidth(), Random::genFloat() * Graphics::getHeight(), xSpeed, ySpeed);

				maxSpeedMaxForceArray[i] = DirectX::XMFLOAT2(3.f, 0.1f);
			}

			positionVelocity = ResourceManager::createSwapBuffer(
				[&] {return resManager->createTypedBufferView(FMT::RGBA32F, sizeof(DirectX::XMFLOAT4) * numVehicle, true, true, true, false, false, true, positionVelocityArray.get()); },
				[&] {return ResourceManager::createTypedBufferView(FMT::RGBA32F, sizeof(DirectX::XMFLOAT4) * numVehicle, true, true, true, false, false, true); });

			maxSpeedMaxForce = resManager->createTypedBufferView(FMT::RG32F, sizeof(DirectX::XMFLOAT2) * numVehicle,
				true, false, false, false, false, true, maxSpeedMaxForceArray.get());

			maxSpeedMaxForce->getBuffer()->setName(L"Max Speed Max Force Buffer");
		}

		stepState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"StepCS.cso"));

		vehicleRenderState = PipelineStateBuilder()
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setBlendState(PipelineStateHelper::blendDefault)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setVS(*vehicleVS)
			.setGS(*vehicleGS)
			.setPS(*vehiclePS)
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
		simulationParam.mousePos = DirectX::XMFLOAT2(Mouse::getX(), Mouse::getY());

		context->setPipelineState(*stepState);

		SETCONSTS({
		context->setCSConstants({
			positionVelocity->getUAVIndex(),
			positionVelocity->getSRVIndex(),
			maxSpeedMaxForce->getSRVIndex() }, co);

		context->setCSConstants(simulationParam, co);
			});

		context->dispatchDim(numVehicle, 1, 1);

		positionVelocity->swap();

		context->setPipelineState(*vehicleRenderState);

		context->setRenderTargets({ renderTexture->getRTVMip(0) });

		context->clearRenderTarget(renderTexture->getRTVMip(0), DirectX::Colors::White);

		context->setViewportSimple(renderTexture->get2Dimension());

		context->setPrimitiveTopology(TOPOLOGY::POINTLIST);

		context->setVertexBuffers({ *positionVelocity }, 0);

		SETCONSTS({
		context->setPSConstants({ arrowTexture->getAllSRVIndex() }, co);
			});

		context->draw(numVehicle, 1, 0, 0);

		blit(*renderTexture);
	}

private:

	SwapBufferPtr positionVelocity;

	BufferViewPtr maxSpeedMaxForce;

	RenderTextureViewPtr renderTexture;

	RenderTextureViewPtr arrowTexture;

	struct SimulationParam
	{
		uint32_t numVehicle;
		float speedMultiply;
		DirectX::XMFLOAT2 mousePos;
		float fleeRadius = 75.f;
	} simulationParam;

	static constexpr uint32_t numVehicle = 1000;

	ComputeStatePtr stepState;

	GraphicsStatePtr vehicleRenderState;

	ShaderPtr vehicleVS;

	ShaderPtr vehicleGS;

	ShaderPtr vehiclePS;

};
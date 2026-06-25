#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

#include"Scene.h"

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		scene("E:/Assets/PBRRendering/DNA.obj", *resManager, FMT::RGBA16F),
		skyboxPS(Shader::create(File::getRootFolder() + L"SkyboxPS.cso")),
		prefilterVS(Shader::create(File::getRootFolder() + L"PrefilterVS.cso")),
		prefilterPS(Shader::create(File::getRootFolder() + L"PrefilterPS.cso")),
		irradiancePS(Shader::create(File::getRootFolder() + L"IrradiancePS.cso"))
	{
		renderTexture = ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA16F, 1, 1, false, true, DirectX::Colors::Black);

		depthTexture = ResourceManager::createDepthTextureView(Graphics::getWidth(), Graphics::getHeight(), FMT::D32F, 1, 1, false, true);

		brdfLUTTexture = resManager->createRenderTextureView(L"BRDFLUT.dds", true);

		brdfLUTTexture->getTexture()->setName(L"BRDFLUT");

		envCube = resManager->createTextureCube(L"E:\\Assets\\sundowner_deck_4k.hdr", 1024, true);

		skyboxState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLessEqual)
			.setVS(*GlobalShader::getTextureCubeVS())
			.setPS(*skyboxPS)
			.build();

		prefilterCube = ResourceManager::createGraphicsTexture(1024, 1024, FMT::RGBA16F, 6, 6, true, true);

		prefilterCube->getTexture()->setName(L"Prefilter Cube");

		{
			struct Matrices
			{
				DirectX::XMMATRIX matrices[6];
				DirectX::XMFLOAT4 padding[8];
			} matrices{};

			{
				const DirectX::XMVECTOR focusPoints[6] =
				{
					{1.0f,  0.0f,  0.0f},
					{-1.0f,  0.0f,  0.0f},
					{0.0f,  1.0f,  0.0f},
					{0.0f, -1.0f,  0.0f},
					{0.0f,  0.0f,  1.0f},
					{0.0f,  0.0f, -1.0f}
				};
				const DirectX::XMVECTOR upVectors[6] =
				{
					{0.0f, 1.0f,  0.0f},
					{0.0f, 1.0f,  0.0f},
					{0.0f,  0.0f,  -1.0f},
					{0.0f,  0.0f, 1.0f},
					{0.0f, 1.0f,  0.0f},
					{0.0f, 1.0f,  0.0f}
				};

				const DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(Math::halfPi, 1.f, 0.1f, 10.f);

				for (uint32_t i = 0; i < 6; i++)
				{
					const DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH({ 0.f,0.f,0.f }, focusPoints[i], upVectors[i]);
					const DirectX::XMMATRIX viewProj = DirectX::XMMatrixTranspose(viewMatrix * projMatrix);

					matrices.matrices[i] = viewProj;
				}
			}

			viewProjMatrixBuffer = resManager->createImmutableCBuffer(sizeof(Matrices), &matrices, false);
		}

		prefilterState = PipelineStateBuilder()
			.setVS(*prefilterVS)
			.setPS(*prefilterPS)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setBlendState(PipelineStateHelper::blendReplace)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.build();

		irradianceCube = ResourceManager::createGraphicsTexture(128, 128, FMT::RGBA16F, 6, 1, true, true);

		irradianceCube->getTexture()->setName(L"Irradiance Cube");

		irradianceState = PipelineStateBuilder()
			.setVS(*prefilterVS)
			.setPS(*irradiancePS)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setBlendState(PipelineStateHelper::blendReplace)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.build();

		calcPrefilterCube();

		calcIrradianceCube();

		Graphics::setExposure(0.5f);
	}

	~MyRenderTask()
	{
	}

protected:

	void calcPrefilterCube()
	{
		for (uint32_t i = 0; i < prefilterCube->getTexture()->getMipLevels(); i++)
		{
			const uint32_t width = (prefilterCube->getTexture()->getWidth() >> i);

			const uint32_t height = (prefilterCube->getTexture()->getHeight() >> i);

			context->setPipelineState(*prefilterState);

			context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

			context->setViewportSimple(width, height);

			context->setRenderTargets({ prefilterCube->getRTVMip(i) });

			context->setVSConstantBuffer(*viewProjMatrixBuffer);

			const float roughness = static_cast<float>(i) / static_cast<float>(prefilterCube->getTexture()->getMipLevels() - 1u);

			SETCONSTS({
			context->setPSConstants({ envCube->getAllSRVIndex() }, co);

			context->setPSConstants(1, &roughness, co);
				});

			context->draw(36, 6, 0, 0);
		}
	}

	void calcIrradianceCube()
	{
		context->setPipelineState(*irradianceState);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setViewportSimple(irradianceCube->get2Dimension());

		context->setRenderTargets({ irradianceCube->getRTVMip(0) });

		context->setVSConstantBuffer(*viewProjMatrixBuffer);

		SETCONSTS({
		context->setPSConstants({ envCube->getAllSRVIndex() }, co);
			});

		context->draw(36, 6, 0, 0);
	}

	void recordCommand() override
	{
		context->setRenderTargets({ renderTexture->getRTVMip(0) }, depthTexture->getDSVMip(0));

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->clearRenderTarget(renderTexture->getRTVMip(0), DirectX::Colors::Black);

		context->clearDepthStencil(CLEARFLAG::DEPTH, 1.0f, 0);

		//基本物体
		scene.draw(context, *prefilterCube, *brdfLUTTexture, *irradianceCube);

		//天空盒
		context->setPipelineState(*skyboxState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ renderTexture->getRTVMip(0) }, depthTexture->getDSVMip(0));

		SETCONSTS({
		context->setPSConstants({ envCube->getAllSRVIndex() }, co);
			});

		context->draw(36, 1, 0, 0);

		auto toneMappedTexture = ToneMapEffect::process(*context, *renderTexture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(*context, *toneMappedTexture);

		blit(*gammaCorrectedTexture);
	}

	void imGuiCall() override
	{
		ImGui::Begin("Scene Info");
		ImGui::SliderFloat("Metallic", &scene.sceneInfo.metallic, 0.f, 1.f);
		ImGui::SliderFloat("Roughness", &scene.sceneInfo.roughness, 0.f, 1.f);
		ImGui::Checkbox("Enable Specular IBL", (bool*)&scene.sceneInfo.enableSpecularIBL);
		ImGui::Checkbox("Enable Diffuse IBL", (bool*)&scene.sceneInfo.enableDiffuseIBL);
		ImGui::End();
	}

private:

	Scene scene;

	RenderTextureViewPtr renderTexture;

	DepthTextureViewPtr depthTexture;

	RenderTextureViewPtr envCube;

	RenderTextureViewPtr irradianceCube;

	RenderTextureViewPtr prefilterCube;

	RenderTextureViewPtr brdfLUTTexture;

	ShaderPtr skyboxPS;

	GraphicsStatePtr skyboxState;

	ShaderPtr irradiancePS;

	GraphicsStatePtr irradianceState;

	ShaderPtr prefilterVS;

	ShaderPtr prefilterPS;

	GraphicsStatePtr prefilterState;

	//使用SV_RenderTargetIndex一次性绘制到六个面上分别需要六个对应的矩阵
	ImmutableCBufferPtr viewProjMatrixBuffer;

};
#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

#include"Scene.h"

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		scene("E:/Assets/PBRRendering/DNA.obj", resManager, FMT::RGBA16F),
		skyboxPS(Shader::create(Utils::File::getRootFolder() + L"SkyboxPS.cso")),
		prefilterVS(Shader::create(Utils::File::getRootFolder() + L"PrefilterVS.cso")),
		prefilterPS(Shader::create(Utils::File::getRootFolder() + L"PrefilterPS.cso")),
		irradiancePS(Shader::create(Utils::File::getRootFolder() + L"IrradiancePS.cso"))
	{
		renderTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F, DirectX::Colors::Black);

		depthTexture = ResourceManager::createTextureDepthView(Graphics::getWidth(), Graphics::getHeight(), FMT::D32F, 1, 1, false, true);

		brdfLUTTexture = resManager->createTextureRenderView(L"BRDFLUT.dds", true);

		brdfLUTTexture->getTexture()->setName(L"BRDFLUT");

		envCube = resManager->createTextureCube(L"E:\\Assets\\sundowner_deck_4k.hdr", 1024, true);

		skyboxState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLessEqual)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRTVFormats({ FMT::RGBA16F })
			.setDSVFormat(FMT::D32F)
			.setVS(GlobalShader::getTextureCubeVS())
			.setPS(skyboxPS)
			.build();

		prefilterCube = ResourceManager::createTextureRenderView(1024, 1024, FMT::RGBA16F, 6, 6, true, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F);

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

				const DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(Utils::Math::halfPi, 1.f, 0.1f, 10.f);

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
			.setVS(prefilterVS)
			.setPS(prefilterPS)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setBlendState(PipelineStateHelper::blendReplace)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRTVFormats({ FMT::RGBA16F })
			.build();

		irradianceCube = ResourceManager::createTextureRenderView(128, 128, FMT::RGBA16F, 6, 1, true, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F);

		irradianceCube->getTexture()->setName(L"Irradiance Cube");

		irradianceState = PipelineStateBuilder()
			.setVS(prefilterVS)
			.setPS(irradiancePS)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setBlendState(PipelineStateHelper::blendReplace)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRTVFormats({ FMT::RGBA16F })
			.build();

		calcPrefilterCube();

		calcIrradianceCube();

		Graphics::setExposure(0.5f);
	}

	~MyRenderTask()
	{
		if (renderTexture)
			delete renderTexture;

		if (depthTexture)
			delete depthTexture;

		if (envCube)
			delete envCube;

		if (irradianceCube)
			delete irradianceCube;

		if (prefilterCube)
			delete prefilterCube;

		if (skyboxPS)
			delete skyboxPS;

		if (skyboxState)
			delete skyboxState;

		if (viewProjMatrixBuffer)
			delete viewProjMatrixBuffer;

		if (irradiancePS)
			delete irradiancePS;

		if (irradianceState)
			delete irradianceState;

		if (prefilterVS)
			delete prefilterVS;

		if (prefilterPS)
			delete prefilterPS;

		if (prefilterState)
			delete prefilterState;

		if (brdfLUTTexture)
			delete brdfLUTTexture;

	}

protected:

	void calcPrefilterCube()
	{
		for (uint32_t i = 0; i < prefilterCube->getTexture()->getMipLevels(); i++)
		{
			const uint32_t width = (prefilterCube->getTexture()->getWidth() >> i);

			const uint32_t height = (prefilterCube->getTexture()->getHeight() >> i);

			context->setPipelineState(prefilterState);

			context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

			context->setViewportSimple(width, height);

			context->setRenderTargets({ prefilterCube->getRTVMipHandle(i) });

			context->setVSConstantBuffer(viewProjMatrixBuffer);

			context->setPSConstants({ envCube->getAllSRVIndex() }, 0);

			const float roughness = static_cast<float>(i) / static_cast<float>(prefilterCube->getTexture()->getMipLevels() - 1u);

			context->setPSConstants(1, &roughness, 1);

			context->draw(36, 6, 0, 0);
		}
	}

	void calcIrradianceCube()
	{
		context->setPipelineState(irradianceState);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setViewportSimple(irradianceCube->getTexture()->getWidth(), irradianceCube->getTexture()->getHeight());

		context->setRenderTargets({ irradianceCube->getRTVMipHandle(0) });

		context->setPSConstants({ envCube->getAllSRVIndex() }, 0);

		context->draw(36, 6, 0, 0);
	}

	void recordCommand() override
	{
		context->setRenderTargets({ renderTexture->getRTVMipHandle(0) }, depthTexture->getDSVMipHandle(0));

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->clearRenderTarget(renderTexture->getRTVMipHandle(0), DirectX::Colors::Black);

		context->clearDepthStencil(depthTexture->getDSVMipHandle(0), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

		//基本物体
		scene.draw(context, prefilterCube, brdfLUTTexture, irradianceCube);

		//天空盒
		context->setPipelineState(skyboxState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ renderTexture->getRTVMipHandle(0) }, depthTexture->getDSVMipHandle(0));

		context->setPSConstants({ envCube->getAllSRVIndex() }, 0);

		context->draw(36, 1, 0, 0);

		auto toneMappedTexture = ToneMapEffect::process(context, renderTexture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(context, toneMappedTexture);

		blit(gammaCorrectedTexture);
	}

	void imGUICall() override
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

	TextureRenderView* renderTexture;

	TextureDepthView* depthTexture;

	TextureRenderView* envCube;

	TextureRenderView* irradianceCube;

	TextureRenderView* prefilterCube;

	TextureRenderView* brdfLUTTexture;

	Shader* skyboxPS;

	PipelineState* skyboxState;

	Shader* irradiancePS;

	PipelineState* irradianceState;

	Shader* prefilterVS;

	Shader* prefilterPS;

	PipelineState* prefilterState;

	//使用SV_RenderTargetIndex一次性绘制到六个面上分别需要六个对应的矩阵
	ImmutableCBuffer* viewProjMatrixBuffer;

};
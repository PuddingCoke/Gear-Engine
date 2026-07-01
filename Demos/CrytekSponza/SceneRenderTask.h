#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

#include"Scene.h"

class SceneRenderTask :public RenderTask
{
public:

	SceneRenderTask(const RenderTextureView& ssrCombinedTexture) :
		gPositionMetallic(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA32F, 1, 1, false, true, DirectX::g_XMHalfPi)),
		gNormalRoughness(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA32F, 1, 1, false, true, DirectX::Colors::Transparent)),
		gBaseColor(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA8UN, 1, 1, false, true, DirectX::Colors::Transparent)),
		depthTexture(ResourceManager::createDepthTextureView(Graphics::getWidth(), Graphics::getHeight(),
			FMT::R32TL, 1, 1, false, true)),
		shadowTexture(ResourceManager::createDepthTextureView(shadowTextureResolution, shadowTextureResolution,
			FMT::R32TL, 1, 1, false, true)),
		originTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA16F, 1, 1, false, true, DirectX::Colors::Black)),
		ssrCombinedTexture(makeUnique<RenderTextureView>(ssrCombinedTexture)),
		radianceCube(ResourceManager::createGraphicsTexture(probeCaptureResolution, probeCaptureResolution,
			FMT::RGBA16F, 6, 1, true, true, radianceCubeClearColor)),
		distanceCube(ResourceManager::createGraphicsTexture(probeCaptureResolution, probeCaptureResolution,
			FMT::R32F, 6, 1, true, true, distanceCubeClearColor)),
		depthCube(ResourceManager::createDepthTextureView(probeCaptureResolution, probeCaptureResolution,
			FMT::D32F, 6, 1, true, true)),
		irradianceVolumeBuffer(ResourceManager::createDefaultCBuffer(sizeof(IrradianceVolume), true)),
		shadowVS(Shader::create(File::getRootFolder() + L"ShadowVS.cso")),
		deferredVShader(Shader::create(File::getRootFolder() + L"DeferredVShader.cso")),
		deferredPShader(Shader::create(File::getRootFolder() + L"DeferredPShader.cso")),
		deferredFinal(Shader::create(File::getRootFolder() + L"DeferredFinal.cso")),
		cubeRenderVS(Shader::create(File::getRootFolder() + L"CubeRenderVS.cso")),
		cubeRenderPS(Shader::create(File::getRootFolder() + L"CubeRenderPS.cso")),
		cubeRenderBouncePS(Shader::create(File::getRootFolder() + L"CubeRenderBouncePS.cso")),
		skyboxPShader(Shader::create(File::getRootFolder() + L"SkybosPShader.cso")),
		ssrCombinePS(Shader::create(File::getRootFolder() + L"SSRCombinePS.cso")),
		sunAngle(Math::halfPi - 0.01f)
	{
		shadowTexture->getTexture()->setName(L"Shadow Texture");

		radianceCube->getTexture()->setName(L"Radiance Cube");

		distanceCube->getTexture()->setName(L"Distance Cube");

		originTexture->getTexture()->setName(L"Origin Texture");

		shadowPipelineState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterShadow)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setVS(*shadowVS)
			.build();

		deferredPipelineState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setVS(*deferredVShader)
			.setPS(*deferredPShader)
			.build();

		deferredFinalPipelineState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setPS(*deferredFinal)
			.build();

		probeCapturePipelineState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setVS(*cubeRenderVS)
			.setPS(*cubeRenderPS)
			.build();

		probeCaptureBouncePipelineState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setVS(*cubeRenderVS)
			.setPS(*cubeRenderBouncePS)
			.build();

		skyboxState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLessEqual)
			.setVS(*GlobalShader::getTextureCubeVS())
			.setPS(*skyboxPShader)
			.build();

		ssrCombineState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setVS(*GlobalShader::getFullScreenVS())
			.setPS(*ssrCombinePS)
			.build();

		irradianceOctahedralEncodeState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"IrradianceOctahedralEncode.cso"));

		depthOctahedralEncodeState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"DepthOctahedralEncode.cso"));

		skybox = resManager->createTextureCube(L"E:/Assets/Sponza/sky/kloppenheim_05_4k.hdr", 1024, true);

		{
			struct CubeRenderParam
			{
				DirectX::XMMATRIX viewProj[6];
				DirectX::XMFLOAT3 probeLocation;
				uint32_t probeIndex;
				DirectX::XMFLOAT4 padding[7];
			} cubeRenderParam{};

			for (uint32_t x = 0; x < irradianceVolume.count.x; x++)
			{
				for (uint32_t z = 0; z < irradianceVolume.count.z; z++)
				{
					for (uint32_t y = 0; y < irradianceVolume.count.y; y++)
					{
						const DirectX::XMFLOAT3 location = ProbeGridPosToLoc({ x,y,z });

						const uint32_t probeIndex = ProbeGridPosToIndex({ x,y,z });

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

						const DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(Math::pi / 2.f, 1.f, 1.f, 512.f);

						for (int i = 0; i < 6; i++)
						{
							const DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&location), DirectX::XMVectorAdd(focusPoints[i], DirectX::XMLoadFloat3(&location)), upVectors[i]);
							cubeRenderParam.viewProj[i] = DirectX::XMMatrixTranspose(viewMatrix * projMatrix);
						}

						cubeRenderParam.probeLocation = location;
						cubeRenderParam.probeIndex = probeIndex;

						cubeRenderParamBuffer[probeIndex] = resManager->createImmutableCBuffer(sizeof(CubeRenderParam), &cubeRenderParam, true);
					}
				}
			}
		}

		hbaoPlusEffect = HBAOPlusEffect::create(*context, Graphics::getWidth(), Graphics::getHeight());

		ssrEffect = SSREffect::create(*context, Graphics::getWidth(), Graphics::getHeight());

		scene = makeUnique<Scene>(assetPath + "Sponza.gltf", *resManager);

		irradianceOctahedralMap = ResourceManager::createComputeTexture(6, 6, FMT::RG11B10F, probeCount, 1, false, true);

		const bool irradianceDataExist = File::exist(L"Irradiance_Bounce_Octahedral_Map.dds");

		irradianceBounceOctahedralMap = irradianceDataExist ? resManager->createRenderTextureView(L"Irradiance_Bounce_Octahedral_Map.dds", true, true, false) :
			ResourceManager::createComputeTexture(6, 6, FMT::RG11B10F, probeCount, 1, false, true);

		const bool depthDataExist = File::exist(L"Depth_Octahedral_Map.dds");

		depthOctahedralMap = depthDataExist ? resManager->createRenderTextureView(L"Depth_Octahedral_Map.dds", true, true, false) :
			ResourceManager::createComputeTexture(16, 16, FMT::RG16F, probeCount, 1, false, true);

		irradianceOctahedralMap->getTexture()->setName(L"Irradiance Octahedral Map");

		irradianceBounceOctahedralMap->getTexture()->setName(L"Irradiance Bounce Octahedral Map");

		depthOctahedralMap->getTexture()->setName(L"Depth Octahedral Map");

		updateShadow();

		if (!irradianceDataExist && !depthDataExist)
		{
			LOGUSER(L"未侦测到预烘焙数据，开始计算辐照度场与距离场");

			updateLightProbe();
		}
		else
		{
			LOGUSER(L"侦测到预烘焙数据，直接读取对应文件");
		}
	}

	~SceneRenderTask()
	{
	}

	void imGuiCall() override
	{
		hbaoPlusEffect->imGuiCall();

		ImGui::Begin("SSR Parameters");
		ImGui::SliderFloat("ExponentA", &ssrParameters.exponentA, 0.f, 5.f);
		ImGui::SliderFloat("ExponentB", &ssrParameters.exponentB, 0.f, 5.f);
		ImGui::End();

		ImGui::Begin("Clip Parameters");
		ImGui::SliderFloat("Clip Max Distance", &clipParameters.clipMaxDistance, 0.f, 2048.f);
		ImGui::SliderFloat("Clip Exponent", &clipParameters.clipExponent, 0.f, 1.f);
		ImGui::End();
	}

protected:

	static constexpr uint32_t shadowTextureResolution = 4096;

	static constexpr uint32_t probeCaptureResolution = 64;

	static constexpr float radianceCubeClearColor[4] = { 0.f,0.f,0.f,1.f };

	static constexpr float distanceCubeClearColor[4] = { 512.f,512.f,512.f,512.f };

	void updateLightField()
	{
		updateShadow();

		updateLightProbe();
	}

	void updateShadow()
	{
		const float xSize = 183;
		const float ySize = 130;
		const float distance = 260.f;
		const DirectX::XMVECTOR offset = { 6.5f,0.f,0.f };

		irradianceVolume.lightDir = { 0.f,sinf(sunAngle),cosf(sunAngle),0.f };

		irradianceVolume.lightColor = DirectX::XMVectorScale(DirectX::Colors::White, 13.5f);

		irradianceVolume.lightDir = DirectX::XMVector3Normalize(irradianceVolume.lightDir);

		const DirectX::XMVECTOR lightCamPos = DirectX::XMVectorAdd(DirectX::XMVectorScale(irradianceVolume.lightDir, distance), offset);
		const DirectX::XMMATRIX lightProjMat = DirectX::XMMatrixOrthographicLH(xSize, ySize, 1.f, 512.f);
		const DirectX::XMMATRIX lightViewMat = DirectX::XMMatrixLookAtLH(lightCamPos, offset, { 0.f,1.f,0.f });
		const DirectX::XMMATRIX lightMat = DirectX::XMMatrixTranspose(lightViewMat * lightProjMat);

		irradianceVolume.lightViewProj = lightMat;

		context->updateBuffer(*irradianceVolumeBuffer, &irradianceVolume, sizeof(IrradianceVolume));

		context->setPipelineState(*shadowPipelineState);

		context->setRenderTargets(shadowTexture->getDSVMip(0));

		context->setViewportSimple(shadowTextureResolution, shadowTextureResolution);

		context->setVSConstantBuffer(*irradianceVolumeBuffer);

		context->clearDepthStencil(CLEARFLAG::DEPTH, 1.0f, 0);

		scene->render(context);
	}

	void updateLightProbe()
	{
		for (uint32_t i = 0; i < probeCount; i++)
		{
			renderCubeAt(*cubeRenderParamBuffer[i]);
		}

		for (uint32_t i = 0; i < probeCount; i++)
		{
			renderCubeBounceAt(*cubeRenderParamBuffer[i]);
		}
	}

	void renderCubeAt(const ImmutableCBuffer& cubeRenderBuffer)
	{
		context->setPipelineState(*probeCapturePipelineState);

		context->setViewportSimple(probeCaptureResolution, probeCaptureResolution);

		context->setRenderTargets({ radianceCube->getRTVMip(0),distanceCube->getRTVMip(0) }, depthCube->getDSVMip(0));

		context->setVSConstantBuffer(cubeRenderBuffer);

		SETCONSTS({
		co = 3;
		context->setPSConstants({
			shadowTexture->getAllDepthIndex(),
			irradianceVolumeBuffer->getBufferIndex()
			}, co);
			});

		context->setPSConstantBuffer(cubeRenderBuffer);

		context->clearRenderTarget(radianceCube->getRTVMip(0), radianceCubeClearColor);

		context->clearRenderTarget(distanceCube->getRTVMip(0), distanceCubeClearColor);

		context->clearDepthStencil(CLEARFLAG::DEPTH, 1.0f, 0);

		scene->renderCube(context);

		context->setPipelineState(*irradianceOctahedralEncodeState);

		SETCONSTS({
		context->setCSConstants({
			irradianceOctahedralMap->getUAVMipIndex(0),
			radianceCube->getAllSRVIndex()
			}, co);
			});

		context->setCSConstantBuffer(cubeRenderBuffer);

		context->dispatchDim(irradianceOctahedralMap->get3Dimension());

		context->setPipelineState(*depthOctahedralEncodeState);

		SETCONSTS({
		context->setCSConstants({
			depthOctahedralMap->getUAVMipIndex(0),
			distanceCube->getAllSRVIndex()
			}, co);
			});

		context->setCSConstantBuffer(cubeRenderBuffer);

		context->dispatchDim(depthOctahedralMap->get3Dimension());
	}

	void renderCubeBounceAt(const ImmutableCBuffer& cubeRenderBuffer)
	{
		context->setPipelineState(*probeCaptureBouncePipelineState);

		context->setViewportSimple(probeCaptureResolution, probeCaptureResolution);

		context->setRenderTargets({ radianceCube->getRTVMip(0) }, depthCube->getDSVMip(0));

		context->setVSConstantBuffer(cubeRenderBuffer);

		SETCONSTS({
		co = 3;
		context->setPSConstants({
			shadowTexture->getAllDepthIndex(),
			irradianceVolumeBuffer->getBufferIndex(),
			irradianceOctahedralMap->getAllSRVIndex(),
			depthOctahedralMap->getAllSRVIndex()
			}, co);
			});

		context->setPSConstantBuffer(cubeRenderBuffer);

		context->clearRenderTarget(radianceCube->getRTVMip(0), radianceCubeClearColor);

		context->clearDepthStencil(CLEARFLAG::DEPTH, 1.0f, 0);

		scene->renderCube(context);

		context->setPipelineState(*irradianceOctahedralEncodeState);

		SETCONSTS({
		context->setCSConstants({
			irradianceBounceOctahedralMap->getUAVMipIndex(0),
			radianceCube->getAllSRVIndex()
			}, co);
			});

		context->setCSConstantBuffer(cubeRenderBuffer);

		context->dispatchDim(irradianceBounceOctahedralMap->get3Dimension());
	}

	void recordCommand() override
	{
		context->setPipelineState(*deferredPipelineState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setRenderTargets({
			gPositionMetallic->getRTVMip(0),
			gNormalRoughness->getRTVMip(0),
			gBaseColor->getRTVMip(0)
			}, depthTexture->getDSVMip(0));

		SETCONSTS({
		co = 3;
		context->setPSConstants(clipParameters, co);
			});

		context->clearRenderTarget(gPositionMetallic->getRTVMip(0), DirectX::g_XMHalfPi);

		context->clearRenderTarget(gNormalRoughness->getRTVMip(0), DirectX::Colors::Transparent);

		context->clearRenderTarget(gBaseColor->getRTVMip(0), DirectX::Colors::Transparent);

		context->clearDepthStencil(CLEARFLAG::DEPTH, 1.0f, 0);

		scene->render(context);

		RenderTextureView* const aoTexture = hbaoPlusEffect->process(*depthTexture, *gNormalRoughness);

		context->setPipelineState(*deferredFinalPipelineState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ originTexture->getRTVMip(0) });

		SETCONSTS({
		context->setPSConstants({
			gPositionMetallic->getAllSRVIndex(),
			gNormalRoughness->getAllSRVIndex(),
			gBaseColor->getAllSRVIndex(),
			shadowTexture->getAllDepthIndex(),
			irradianceBounceOctahedralMap->getAllSRVIndex(),
			depthOctahedralMap->getAllSRVIndex(),
			aoTexture->getAllSRVIndex()
			}, co);
			});

		context->setPSConstantBuffer(*irradianceVolumeBuffer);

		context->clearRenderTarget(originTexture->getRTVMip(0), DirectX::Colors::Black);

		context->drawQuad();

		context->setPipelineState(*skyboxState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ originTexture->getRTVMip(0) }, depthTexture->getDSVMip(0));

		SETCONSTS({
		context->setPSConstants({ skybox->getAllSRVIndex() }, co);
			});

		context->draw(36, 1, 0, 0);

		RenderTextureView* const ssrUVVisibilityTexture = ssrEffect->process(*depthTexture, *gPositionMetallic, *gNormalRoughness);

		context->setPipelineState(*ssrCombineState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ ssrCombinedTexture->getRTVMip(0) });

		SETCONSTS({
		context->setPSConstants({ originTexture->getAllSRVIndex(),ssrUVVisibilityTexture->getAllSRVIndex(),gNormalRoughness->getAllSRVIndex() }, co);

		context->setPSConstants(ssrParameters, co);
			});

		context->clearRenderTarget(ssrCombinedTexture->getRTVMip(0), DirectX::Colors::Black);

		context->drawQuad();
	}

	uint32_t ProbeGridPosToIndex(const DirectX::XMUINT3& probeGridPos)
	{
		return probeGridPos.x + probeGridPos.z * irradianceVolume.count.x + probeGridPos.y * irradianceVolume.count.x * irradianceVolume.count.z;
	}

	DirectX::XMFLOAT3 ProbeGridPosToLoc(const DirectX::XMUINT3& probeGridPos)
	{
		return {
			probeGridPos.x * irradianceVolume.spacing + irradianceVolume.start.x,
			probeGridPos.y * irradianceVolume.spacing + irradianceVolume.start.y,
			probeGridPos.z * irradianceVolume.spacing + irradianceVolume.start.z,
		};
	}

	UniquePtr<Scene> scene;

	static constexpr uint32_t probeCountX = 17;

	static constexpr uint32_t probeCountY = 9;

	static constexpr uint32_t probeCountZ = 12;

	static constexpr uint32_t probeCount = probeCountX * probeCountY * probeCountZ;

	struct IrradianceVolume
	{
		DirectX::XMVECTOR lightDir;
		DirectX::XMVECTOR lightColor;
		DirectX::XMMATRIX lightViewProj;
		DirectX::XMFLOAT3 start = { -142.f,-16.f,-74.f };
		float spacing = 18.2f;
		const DirectX::XMUINT3 count = { probeCountX,probeCountY,probeCountZ };
		float irradianceDistanceBias = 0.f;
		float irradianceVarianceBias = 0.f;
		float irradianceChebyshevBias = 0.0f;
		DirectX::XMFLOAT2 padding0;
		DirectX::XMFLOAT4 padding1[7];
	} irradianceVolume;

	struct SSRParamaters
	{
		float exponentA = 1.f;
		float exponentB = 2.f;
	}ssrParameters;

	struct ClipParameters
	{
		float clipMaxDistance = 1024.f;
		float clipExponent = 0.3f;
	}clipParameters;

	DefaultCBufferPtr irradianceVolumeBuffer;

	ImmutableCBufferPtr cubeRenderParamBuffer[probeCount];

	float sunAngle;

	RenderTextureViewPtr gPositionMetallic;

	RenderTextureViewPtr gNormalRoughness;

	RenderTextureViewPtr gBaseColor;

	DepthTextureViewPtr depthTexture;

	DepthTextureViewPtr shadowTexture;

	RenderTextureViewPtr originTexture;

	RenderTextureViewPtr ssrCombinedTexture;

	RenderTextureViewPtr radianceCube;

	RenderTextureViewPtr distanceCube;

	DepthTextureViewPtr depthCube;

	RenderTextureViewPtr irradianceOctahedralMap;

	RenderTextureViewPtr irradianceBounceOctahedralMap;

	RenderTextureViewPtr depthOctahedralMap;

	RenderTextureViewPtr skybox;

	GraphicsStatePtr shadowPipelineState;

	GraphicsStatePtr deferredPipelineState;

	GraphicsStatePtr deferredFinalPipelineState;

	GraphicsStatePtr probeCapturePipelineState;

	GraphicsStatePtr probeCaptureBouncePipelineState;

	ComputeStatePtr irradianceOctahedralEncodeState;

	ComputeStatePtr depthOctahedralEncodeState;

	GraphicsStatePtr skyboxState;

	GraphicsStatePtr ssrCombineState;

	ShaderPtr shadowVS;

	ShaderPtr deferredVShader;

	ShaderPtr deferredPShader;

	ShaderPtr deferredFinal;

	ShaderPtr cubeRenderVS;

	ShaderPtr cubeRenderPS;

	ShaderPtr cubeRenderBouncePS;

	ShaderPtr skyboxPShader;

	ShaderPtr ssrCombinePS;

	HBAOPlusEffectPtr hbaoPlusEffect;

	SSREffectPtr ssrEffect;

};
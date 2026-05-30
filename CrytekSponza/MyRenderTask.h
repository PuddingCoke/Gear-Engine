#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/Core/Effect/BloomEffect.h>

#include<Gear/Core/Effect/FXAAEffect.h>

#include<Gear/Core/Effect/SSREffect.h>

#include<Gear/Core/Effect/HBAOPlusEffect.h>

#include<Gear/DevEssential.h>

#include"Scene.h"

#include<iostream>

using namespace Core;

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask() :
		gPositionMetallic(ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA32F, 1, 1, false, true,
			FMT::RGBA32F, FMT::UNKNOWN, FMT::RGBA32F, DirectX::g_XMHalfPi)),
		gNormalRoughness(ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA32F, 1, 1, false, true,
			FMT::RGBA32F, FMT::UNKNOWN, FMT::RGBA32F, DirectX::Colors::Transparent)),
		gBaseColor(ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA8UN, 1, 1, false, true,
			FMT::RGBA8UN, FMT::UNKNOWN, FMT::RGBA8UN, DirectX::Colors::Transparent)),
		depthTexture(ResourceManager::createTextureDepthView(Graphics::getWidth(), Graphics::getHeight(), FMT::R32TL, 1, 1, false, true)),
		shadowTexture(ResourceManager::createTextureDepthView(shadowTextureResolution, shadowTextureResolution, FMT::R32TL, 1, 1, false, true)),
		originTexture(ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F, DirectX::Colors::Black)),
		ssrCombinedTexture(ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F, DirectX::Colors::Black)),
		radianceCube(ResourceManager::createTextureRenderView(probeCaptureResolution, probeCaptureResolution, FMT::RGBA16F, 6, 1, true, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F, radianceCubeClearColor)),
		distanceCube(ResourceManager::createTextureRenderView(probeCaptureResolution, probeCaptureResolution, FMT::R32F, 6, 1, true, true,
			FMT::R32F, FMT::UNKNOWN, FMT::R32F, distanceCubeClearColor)),
		depthCube(ResourceManager::createTextureDepthView(probeCaptureResolution, probeCaptureResolution, FMT::D32F, 6, 1, true, true)),
		irradianceVolumeBuffer(ResourceManager::createStaticCBuffer(sizeof(IrradianceVolume), true)),
		shadowVS(new Shader(Utils::File::getRootFolder() + L"ShadowVS.cso")),
		deferredVShader(new Shader(Utils::File::getRootFolder() + L"DeferredVShader.cso")),
		deferredPShader(new Shader(Utils::File::getRootFolder() + L"DeferredPShader.cso")),
		deferredFinal(new Shader(Utils::File::getRootFolder() + L"DeferredFinal.cso")),
		cubeRenderVS(new Shader(Utils::File::getRootFolder() + L"CubeRenderVS.cso")),
		cubeRenderPS(new Shader(Utils::File::getRootFolder() + L"CubeRenderPS.cso")),
		cubeRenderBouncePS(new Shader(Utils::File::getRootFolder() + L"CubeRenderBouncePS.cso")),
		irradianceOctahedralEncode(new Shader(Utils::File::getRootFolder() + L"IrradianceOctahedralEncode.cso")),
		depthOctahedralEncode(new Shader(Utils::File::getRootFolder() + L"DepthOctahedralEncode.cso")),
		skyboxPShader(new Shader(Utils::File::getRootFolder() + L"SkybosPShader.cso")),
		ssrCombinePS(new Shader(Utils::File::getRootFolder() + L"SSRCombinePS.cso")),
		sunAngle(Utils::Math::halfPi - 0.01f)
	{
		shadowTexture->getTexture()->setName(L"Shadow Texture");

		radianceCube->getTexture()->setName(L"Radiance Cube");

		distanceCube->getTexture()->setName(L"Distance Cube");

		originTexture->getTexture()->setName(L"Origin Texture");

		ssrCombinedTexture->getTexture()->setName(L"SSR Combined Texture");

		shadowPipelineState = PipelineStateBuilder()
			.setInputElements(inputDesc)
			.setBlendState(PipelineStateHelper::blendReplace)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRasterizerState(PipelineStateHelper::rasterShadow)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setRTVFormats()
			.setDSVFormat(FMT::D32F)
			.setVS(shadowVS)
			.build();

		deferredPipelineState = PipelineStateBuilder()
			.setInputElements(inputDesc)
			.setBlendState(PipelineStateHelper::blendReplace)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setRTVFormats({ FMT::RGBA32F,FMT::RGBA32F,FMT::RGBA8UN })
			.setDSVFormat(FMT::D32F)
			.setVS(deferredVShader)
			.setPS(deferredPShader)
			.build();

		deferredFinalPipelineState = PipelineStateBuilder()
			.setDefaultFullScreenState()
			.setRTVFormats({ FMT::RGBA16F })
			.setPS(deferredFinal)
			.build();

		probeCapturePipelineState = PipelineStateBuilder()
			.setInputElements(inputDesc)
			.setBlendState(PipelineStateHelper::blendReplace)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setRTVFormats({ FMT::RGBA16F,FMT::R32F })
			.setDSVFormat(FMT::D32F)
			.setVS(cubeRenderVS)
			.setPS(cubeRenderPS)
			.build();

		probeCaptureBouncePipelineState = PipelineStateBuilder()
			.setInputElements(inputDesc)
			.setBlendState(PipelineStateHelper::blendReplace)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setRTVFormats({ FMT::RGBA16F })
			.setDSVFormat(FMT::D32F)
			.setVS(cubeRenderVS)
			.setPS(cubeRenderBouncePS)
			.build();

		skyboxState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLessEqual)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRTVFormats({ FMT::RGBA16F })
			.setDSVFormat(FMT::D32F)
			.setVS(GlobalShader::getTextureCubeVS())
			.setPS(skyboxPShader)
			.build();

		ssrCombineState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRTVFormats({ FMT::RGBA16F })
			.setVS(GlobalShader::getFullScreenVS())
			.setPS(ssrCombinePS)
			.build();

		irradianceOctahedralEncodeState = PipelineStateBuilder::build(irradianceOctahedralEncode);

		depthOctahedralEncodeState = PipelineStateBuilder::build(depthOctahedralEncode);

		skybox = resManager->createTextureCube(L"E:/Assets/Sponza/sky/kloppenheim_05_4k.hdr", 1024, true);

		{
			struct CubeRenderParam
			{
				DirectX::XMMATRIX viewProj[6];
				DirectX::XMFLOAT3 probeLocation;
				UINT probeIndex;
				DirectX::XMFLOAT4 padding[7];
			} cubeRenderParam{};

			for (UINT x = 0; x < irradianceVolume.count.x; x++)
			{
				for (UINT z = 0; z < irradianceVolume.count.z; z++)
				{
					for (UINT y = 0; y < irradianceVolume.count.y; y++)
					{
						const DirectX::XMFLOAT3 location = ProbeGridPosToLoc({ x,y,z });

						const UINT probeIndex = ProbeGridPosToIndex({ x,y,z });

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

						const DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(Utils::Math::pi / 2.f, 1.f, 1.f, 512.f);

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

		hbaoPlusEffect = new HBAOPlusEffect(context, Graphics::getWidth(), Graphics::getHeight());

		bloomEffect = new BloomEffect(context, Graphics::getWidth(), Graphics::getHeight(), resManager);

		bloomEffect->setIntensity(0.5f);

		fxaaEffect = new FXAAEffect(context, Graphics::getWidth(), Graphics::getHeight());

		ssrEffect = new SSREffect(context, Graphics::getWidth(), Graphics::getHeight());

		scene = new Scene(assetPath + "Sponza.gltf", resManager);

		Graphics::setExposure(0.6f);

		Graphics::setGamma(2.2f);

		bloomEffect->setThreshold(0.f);

		bloomEffect->setSoftThreshold(0.f);

		bloomEffect->setIntensity(0.17f);

		irradianceOctahedralMap = ResourceManager::createTextureRenderView(6, 6, FMT::RG11B10F, probeCount, 1, false, true,
			FMT::RG11B10F, FMT::RG11B10F, FMT::UNKNOWN);

		const bool irradianceDataExist = Utils::File::exist(L"Irradiance_Bounce_Octahedral_Map.dds");

		irradianceBounceOctahedralMap = irradianceDataExist ? resManager->createTextureRenderView(L"Irradiance_Bounce_Octahedral_Map.dds", true, true, false) : ResourceManager::createTextureRenderView(6, 6, FMT::RG11B10F, probeCount, 1, false, true,
			FMT::RG11B10F, FMT::RG11B10F, FMT::UNKNOWN);

		const bool depthDataExist = Utils::File::exist(L"Depth_Octahedral_Map.dds");

		depthOctahedralMap = depthDataExist ? resManager->createTextureRenderView(L"Depth_Octahedral_Map.dds", true, true, false) : ResourceManager::createTextureRenderView(16, 16, FMT::RG16F, probeCount, 1, false, true,
			FMT::RG16F, FMT::RG16F, FMT::UNKNOWN);

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

	~MyRenderTask()
	{
		delete scene;

		delete irradianceVolumeBuffer;

		for (UINT i = 0; i < 17 * 9 * 12; i++)
		{
			delete cubeRenderParamBuffer[i];
		}

		delete gPositionMetallic;
		delete gNormalRoughness;
		delete gBaseColor;
		delete depthTexture;
		delete shadowTexture;
		delete originTexture;
		delete ssrCombinedTexture;

		delete radianceCube;
		delete distanceCube;
		delete depthCube;

		delete irradianceOctahedralMap;
		delete irradianceBounceOctahedralMap;
		delete depthOctahedralMap;

		delete skybox;

		delete shadowVS;
		delete deferredVShader;
		delete deferredPShader;
		delete deferredFinal;
		delete cubeRenderVS;
		delete cubeRenderPS;
		delete cubeRenderBouncePS;
		delete skyboxPShader;
		delete ssrCombinePS;

		delete hbaoPlusEffect;
		delete bloomEffect;
		delete fxaaEffect;
		delete ssrEffect;

		delete shadowPipelineState;

		delete deferredPipelineState;

		delete deferredFinalPipelineState;

		delete probeCapturePipelineState;

		delete probeCaptureBouncePipelineState;

		delete irradianceOctahedralEncodeState;

		delete depthOctahedralEncodeState;

		delete skyboxState;

		delete ssrCombineState;
	}

	void imGUICall() override
	{
		hbaoPlusEffect->imGUICall();

		bloomEffect->imGUICall();

		fxaaEffect->imGUICall();

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

	static constexpr UINT shadowTextureResolution = 4096;

	static constexpr UINT probeCaptureResolution = 64;

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

		context->updateBuffer(irradianceVolumeBuffer, &irradianceVolume, sizeof(IrradianceVolume));

		context->setPipelineState(shadowPipelineState);

		const D3D12Resource::DepthStencilDesc dsDesc = shadowTexture->getDSVMipHandle(0);

		context->setRenderTargets(dsDesc);

		context->setViewport(shadowTextureResolution, shadowTextureResolution);

		context->setScissorRect(0, 0, shadowTextureResolution, shadowTextureResolution);

		context->setVSConstantBuffer(irradianceVolumeBuffer);

		context->clearDepthStencil(dsDesc, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

		scene->render(context);
	}

	void updateLightProbe()
	{
		for (UINT i = 0; i < probeCount; i++)
		{
			renderCubeAt(cubeRenderParamBuffer[i]);
		}

		for (UINT i = 0; i < probeCount; i++)
		{
			renderCubeBounceAt(cubeRenderParamBuffer[i]);
		}
	}

	void renderCubeAt(const ImmutableCBuffer* const cubeRenderBuffer)
	{
		context->setPipelineState(probeCapturePipelineState);

		context->setViewport(probeCaptureResolution, probeCaptureResolution);

		context->setScissorRect(0, 0, probeCaptureResolution, probeCaptureResolution);

		const D3D12Resource::DepthStencilDesc dsDesc = depthCube->getDSVMipHandle(0);

		context->setRenderTargets({ radianceCube->getRTVMipHandle(0),distanceCube->getRTVMipHandle(0) }, dsDesc);

		context->setVSConstantBuffer(cubeRenderBuffer);

		context->setPSConstants({
			shadowTexture->getAllDepthIndex(),
			irradianceVolumeBuffer->getBufferIndex()
			}, 3);

		context->setPSConstantBuffer(cubeRenderBuffer);

		context->clearRenderTarget(radianceCube->getRTVMipHandle(0), radianceCubeClearColor);

		context->clearRenderTarget(distanceCube->getRTVMipHandle(0), distanceCubeClearColor);

		context->clearDepthStencil(dsDesc, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

		scene->renderCube(context);

		context->setPipelineState(irradianceOctahedralEncodeState);

		context->setCSConstants({
			irradianceOctahedralMap->getUAVMipIndex(0),
			radianceCube->getAllSRVIndex()
			}, 0);

		context->setCSConstantBuffer(cubeRenderBuffer);

		context->dispatch(1, 1, 1);

		context->uavBarrier({ irradianceOctahedralMap->getTexture() });

		context->setPipelineState(depthOctahedralEncodeState);

		context->setCSConstants({
			depthOctahedralMap->getUAVMipIndex(0),
			distanceCube->getAllSRVIndex()
			}, 0);

		context->setCSConstantBuffer(cubeRenderBuffer);

		context->dispatch(1, 1, 1);

		context->uavBarrier({ depthOctahedralMap->getTexture() });
	}

	void renderCubeBounceAt(const ImmutableCBuffer* const cubeRenderBuffer)
	{
		context->setPipelineState(probeCaptureBouncePipelineState);

		context->setViewport(probeCaptureResolution, probeCaptureResolution);

		context->setScissorRect(0, 0, probeCaptureResolution, probeCaptureResolution);

		const D3D12Resource::DepthStencilDesc dsDesc = depthCube->getDSVMipHandle(0);

		context->setRenderTargets({ radianceCube->getRTVMipHandle(0) }, dsDesc);

		context->setVSConstantBuffer(cubeRenderBuffer);

		context->setPSConstants({
			shadowTexture->getAllDepthIndex(),
			irradianceVolumeBuffer->getBufferIndex(),
			irradianceOctahedralMap->getAllSRVIndex(),
			depthOctahedralMap->getAllSRVIndex()
			}, 3);

		context->setPSConstantBuffer(cubeRenderBuffer);

		context->clearRenderTarget(radianceCube->getRTVMipHandle(0), radianceCubeClearColor);

		context->clearDepthStencil(dsDesc, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

		scene->renderCube(context);

		context->setPipelineState(irradianceOctahedralEncodeState);

		context->setCSConstants({
			irradianceBounceOctahedralMap->getUAVMipIndex(0),
			radianceCube->getAllSRVIndex()
			}, 0);

		context->setCSConstantBuffer(cubeRenderBuffer);

		context->dispatch(1, 1, 1);

		context->uavBarrier({ irradianceBounceOctahedralMap->getTexture() });
	}

	void recordCommand() override
	{
		context->setPipelineState(deferredPipelineState);

		context->setViewport(Graphics::getWidth(), Graphics::getHeight());

		context->setScissorRect(0, 0, Graphics::getWidth(), Graphics::getHeight());

		const D3D12Resource::DepthStencilDesc dsDesc = depthTexture->getDSVMipHandle(0);

		context->setRenderTargets({
			gPositionMetallic->getRTVMipHandle(0),
			gNormalRoughness->getRTVMipHandle(0),
			gBaseColor->getRTVMipHandle(0)
			}, dsDesc);

		context->setPSConstants(2, &clipParameters, 3);

		context->clearRenderTarget(gPositionMetallic->getRTVMipHandle(0), DirectX::g_XMHalfPi);

		context->clearRenderTarget(gNormalRoughness->getRTVMipHandle(0), DirectX::Colors::Transparent);

		context->clearRenderTarget(gBaseColor->getRTVMipHandle(0), DirectX::Colors::Transparent);

		context->clearDepthStencil(dsDesc, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

		scene->render(context);

		TextureRenderView* const aoTexture = hbaoPlusEffect->process(depthTexture, gNormalRoughness);

		context->setPipelineState(deferredFinalPipelineState);

		context->setViewport(Graphics::getWidth(), Graphics::getHeight());

		context->setScissorRect(0, 0, Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ originTexture->getRTVMipHandle(0) });

		context->setPSConstants({
			gPositionMetallic->getAllSRVIndex(),
			gNormalRoughness->getAllSRVIndex(),
			gBaseColor->getAllSRVIndex(),
			shadowTexture->getAllDepthIndex(),
			irradianceBounceOctahedralMap->getAllSRVIndex(),
			depthOctahedralMap->getAllSRVIndex(),
			aoTexture->getAllSRVIndex()
			}, 0);

		context->setPSConstantBuffer(irradianceVolumeBuffer);

		context->clearRenderTarget(originTexture->getRTVMipHandle(0), DirectX::Colors::Black);

		context->draw(3, 1, 0, 0);

		context->setPipelineState(skyboxState);

		context->setViewport(Graphics::getWidth(), Graphics::getHeight());

		context->setScissorRect(0, 0, Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ originTexture->getRTVMipHandle(0) }, dsDesc);

		context->setPSConstants({ skybox->getAllSRVIndex() }, 0);

		context->draw(36, 1, 0, 0);

		TextureRenderView* const ssrUVVisibilityTexture = ssrEffect->process(depthTexture, gPositionMetallic, gNormalRoughness);

		context->setPipelineState(ssrCombineState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ ssrCombinedTexture->getRTVMipHandle(0) });

		context->setPSConstants({ originTexture->getAllSRVIndex(),ssrUVVisibilityTexture->getAllSRVIndex(),gNormalRoughness->getAllSRVIndex() }, 0);

		context->setPSConstants(2, &ssrParameters, 3);

		context->clearRenderTarget(ssrCombinedTexture->getRTVMipHandle(0), DirectX::Colors::Black);

		context->draw(3, 1, 0, 0);

		TextureRenderView* const bloomTexture = bloomEffect->process(ssrCombinedTexture);

		TextureRenderView* const toneMappedTexture = ToneMapEffect::process(context, bloomTexture);

		TextureRenderView* const fxaaTexture = fxaaEffect->process(toneMappedTexture);

		TextureRenderView* const gammaCorrectedTexture = GammaCorrectEffect::process(context, fxaaTexture);

		blit(gammaCorrectedTexture);
	}

	UINT ProbeGridPosToIndex(const DirectX::XMUINT3& probeGridPos)
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

	Scene* scene;

	static constexpr UINT probeCountX = 17;

	static constexpr UINT probeCountY = 9;

	static constexpr UINT probeCountZ = 12;

	static constexpr UINT probeCount = probeCountX * probeCountY * probeCountZ;

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

	StaticCBuffer* irradianceVolumeBuffer;

	ImmutableCBuffer* cubeRenderParamBuffer[probeCount];

	float sunAngle;

	TextureRenderView* gPositionMetallic;

	TextureRenderView* gNormalRoughness;

	TextureRenderView* gBaseColor;

	TextureDepthView* depthTexture;

	TextureDepthView* shadowTexture;

	TextureRenderView* originTexture;

	TextureRenderView* ssrCombinedTexture;

	TextureRenderView* radianceCube;

	TextureRenderView* distanceCube;

	TextureDepthView* depthCube;

	TextureRenderView* irradianceOctahedralMap;

	TextureRenderView* irradianceBounceOctahedralMap;

	TextureRenderView* depthOctahedralMap;

	TextureRenderView* skybox;

	PipelineState* shadowPipelineState;

	PipelineState* deferredPipelineState;

	PipelineState* deferredFinalPipelineState;

	PipelineState* probeCapturePipelineState;

	PipelineState* probeCaptureBouncePipelineState;

	PipelineState* irradianceOctahedralEncodeState;

	PipelineState* depthOctahedralEncodeState;

	PipelineState* skyboxState;

	PipelineState* ssrCombineState;

	Shader* shadowVS;

	Shader* deferredVShader;

	Shader* deferredPShader;

	Shader* deferredFinal;

	Shader* cubeRenderVS;

	Shader* cubeRenderPS;

	Shader* cubeRenderBouncePS;

	Shader* irradianceOctahedralEncode;

	Shader* depthOctahedralEncode;

	Shader* skyboxPShader;

	Shader* ssrCombinePS;

	HBAOPlusEffect* hbaoPlusEffect;

	BloomEffect* bloomEffect;

	FXAAEffect* fxaaEffect;

	SSREffect* ssrEffect;

};
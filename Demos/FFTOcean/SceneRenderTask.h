#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

#include"WaveCascade.h"

class SceneRenderTask :public RenderTask
{
public:

	SceneRenderTask(FPSCamera* const camera, const RenderTextureView& originTexture) :
		vertices((gridSize + 1)* (gridSize + 1)),
		camera(camera),
		renderParamBuffer(ResourceManager::createDynamicCBuffer(sizeof(RenderParam))),
		spectrumParamBuffer{ ResourceManager::createDefaultCBuffer(sizeof(SpectrumParam), true),ResourceManager::createDefaultCBuffer(sizeof(SpectrumParam), true),ResourceManager::createDefaultCBuffer(sizeof(SpectrumParam), true) },
		cascade{ makeUnique<WaveCascade>(textureResolution,context),makeUnique<WaveCascade>(textureResolution,context),makeUnique<WaveCascade>(textureResolution,context) },
		textureCubePS(Shader::create(File::getRootFolder() + L"TextureCubePS.cso")),
		gridDebugVS(Shader::create(File::getRootFolder() + L"GridDebugVS.cso")),
		gridDebugPS(Shader::create(File::getRootFolder() + L"GridDebugPS.cso")),
		oceanVS(Shader::create(File::getRootFolder() + L"OceanVS.cso")),
		oceanHS(Shader::create(File::getRootFolder() + L"OceanHS.cso")),
		oceanDS(Shader::create(File::getRootFolder() + L"OceanDS.cso")),
		oceanPS(Shader::create(File::getRootFolder() + L"OceanPS.cso")),
		originTexture(makeUnique<RenderTextureView>(originTexture)),
		renderOcean(true)
	{
		spectrumParam[0].mapLength = lengthScale0;

		spectrumParam[1].mapLength = lengthScale1;

		spectrumParam[2].mapLength = lengthScale2;

		textureCubeState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setVS(*GlobalShader::getTextureCubeVS())
			.setPS(*textureCubePS)
			.build();

		gridDebugState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setVS(*gridDebugVS)
			.setPS(*gridDebugPS)
			.build();

		oceanState = PipelineStateBuilder()
			.setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setVS(*oceanVS)
			.setHS(*oceanHS)
			.setDS(*oceanDS)
			.setPS(*oceanPS)
			.build();

		spectrumState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"SpectrumCS.cso"));

		conjugateState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"ConjugateCS.cso"));

		displacementSpectrumState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"DisplacementSpectrumCS.cso"));

		ifftState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"IFFTCS.cso"));

		permutationState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"PermutationCS.cso"));

		waveMergeState = PipelineStateBuilder::build(Shader::create(File::getRootFolder() + L"WaveMergeCS.cso"));

		tildeh0Texture = createTexture(textureResolution, FMT::RG32F);

		tempTexture = createTexture(textureResolution, FMT::RG32F);

		WaveCascade::spectrumState = spectrumState.get();

		WaveCascade::conjugateState = conjugateState.get();

		WaveCascade::displacementSpectrumState = displacementSpectrumState.get();

		WaveCascade::ifftState = ifftState.get();

		WaveCascade::permutationState = permutationState.get();

		WaveCascade::waveMergeState = waveMergeState.get();

		WaveCascade::tildeh0Texture = tildeh0Texture.get();

		WaveCascade::tempTexture = tempTexture.get();

		depthTexture = ResourceManager::createDepthTextureView(Graphics::getWidth(), Graphics::getHeight(), FMT::R32TL, 1, 1, false, true);

		vertexBuffer = resManager->createStructuredBufferView(sizeof(DirectX::XMFLOAT3), static_cast<uint32_t>(sizeof(DirectX::XMFLOAT3) * vertices.size()), false, false, true, true, true, vertices.data());

		{
			for (uint32_t y = 0; y < gridSize; y++)
			{
				for (uint32_t x = 0; x < gridSize; x++)
				{
					const uint32_t bottomLeft = y * (gridSize + 1) + x;

					const uint32_t bottomRight = bottomLeft + 1;

					const uint32_t topLeft = (y + 1) * (gridSize + 1) + x;

					const uint32_t topRight = topLeft + 1;

					indices.push_back(bottomRight);

					indices.push_back(bottomLeft);

					indices.push_back(topLeft);

					indices.push_back(topLeft);

					indices.push_back(topRight);

					indices.push_back(bottomRight);
				}
			}

			indexBuffer = resManager->createTypedBufferView(FMT::R32UI, sizeof(uint32_t) * indices.size(), false, false, false, true, false, true, indices.data());
		}

		randomGaussTexture = resManager->createRenderTextureView(textureResolution, textureResolution, RandomDataType::GAUSS, true);

		WaveCascade::randomGaussTexture = randomGaussTexture.get();

		enviromentCube = resManager->createTextureCube(L"E:/Assets/Ocean/autumn_field_puresky_4k.exr", 1024, true);

		enviromentCube->getTexture()->setName(L"Enviroment Cube");

		calculateInitialSpectrum();

		tempTexture->getTexture()->setName(L"tempTexture");

		randomGaussTexture->getTexture()->setName(L"randomGaussTexture");

		tildeh0Texture->getTexture()->setName(L"tildeh0Texture");

		depthTexture->getTexture()->setName(L"depthTexture");
	}

	~SceneRenderTask()
	{
	}

	void imGuiCall() override
	{
		ImGui::Begin("RenderParam");
		ImGui::SliderFloat("Lod Scale", &renderParam.lodScale, 1.0f, 10.f);
		ImGui::SliderFloat("Sun Strength", &renderParam.sunStrength, 0.f, 100.f);
		ImGui::SliderFloat("Sun Theta", &renderParam.sunTheta, 0.f, Math::pi / 2.f);
		ImGui::SliderFloat("Specular Pow", &renderParam.specularPower, 0.f, 4096.f);
		ImGui::SliderFloat("Foam Bias", &renderParam.foamBias, 0.f, 5.f);
		ImGui::SliderFloat("Foam Scale", &renderParam.foamScale, 0.f, 1.f);
		ImGui::SliderFloat("Sun Direction", &renderParam.sunDirection, 0.f, Math::twoPi);
		ImGui::End();
	}

protected:

	void recordCommand() override
	{
		renderParam.exposure = Graphics::getExposure();

		renderParam.gamma = Graphics::getGamma();

		updateVertices();

		calculateDisplacementAndDerivative();

		renderSkyDomeAndOceanSurface();
	}

private:

	//不要改动这个值
	static constexpr uint32_t cascadeNum = 3;

	static constexpr float lengthScale0 = 250.f;

	static constexpr float lengthScale1 = 17.f;

	static constexpr float lengthScale2 = 5.f;

	//不要改动这个值
	static constexpr uint32_t textureResolution = 512;

	static constexpr uint32_t gridSize = 128;

	static RenderTextureViewPtr createTexture(const uint32_t& resolution, const DXGI_FORMAT& format)
	{
		return ResourceManager::createComputeTexture(resolution, resolution, format, 1, 1, false, true);
	}

	void calculateInitialSpectrum()
	{
		const float boundary1 = 2.f * Math::pi / lengthScale1 * 12.f;

		const float boundary2 = 2.f * Math::pi / lengthScale2 * 12.f;

		{
			spectrumParam[0].cutoffLow = 0.0001f;

			spectrumParam[0].cutoffHigh = boundary1;
		}

		{
			spectrumParam[1].cutoffLow = boundary1;

			spectrumParam[1].cutoffHigh = boundary2;
		}

		{
			spectrumParam[2].cutoffLow = boundary2;

			spectrumParam[2].cutoffHigh = 9999.f;
		}

		for (uint32_t i = 0; i < cascadeNum; i++)
		{
			context->updateBuffer(*spectrumParamBuffer[i], &spectrumParam[i], sizeof(SpectrumParam));

			cascade[i]->calculateInitialSpectrum(*spectrumParamBuffer[i]);
		}
	}

	void calculateDisplacementAndDerivative()
	{
		if (renderOcean)
		{
			for (uint32_t i = 0; i < cascadeNum; i++)
			{
				cascade[i]->calculateTimeDependentSpectrum();

				cascade[i]->calculateDisplacementAndDerivative();
			}
		}
	}

	void renderSkyDomeAndOceanSurface()
	{
		/*context->setPipelineState(gridDebugState.Get());

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setDefRenderTarget();

		context->setVertexBuffers(0, { vertexBuffer->getVertexBuffer() });

		context->setIndexBuffer(*indexBuffer);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->clearDefRenderTarget(DirectX::Colors::Black);

		context->draw(vertices.size(), 1, 0, 0);

		context->drawIndexed(indices.size(), 1, 0, 0, 0);*/

		renderParamBuffer->simpleUpdate(&renderParam);

		context->setPipelineState(*textureCubeState);

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ originTexture->getRTVMip(0) }, {});

		SETCONSTS({
		context->setPSConstants({
			enviromentCube->getAllSRVIndex() }, co);
			});

		context->draw(36, 1, 0, 0);

		if (renderOcean)
		{
			context->setPipelineState(*oceanState);

			context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

			context->setPrimitiveTopology(TOPOLOGY::PATCH3CONTROL);

			context->setVertexBuffers({ *vertexBuffer }, 0);

			context->setIndexBuffer(*indexBuffer);

			context->setRenderTargets({ originTexture->getRTVMip(0) }, depthTexture->getDSVMip(0));

			SETCONSTS({
			context->setDSConstants({
				cascade[0]->displacementTexture->getAllSRVIndex(),
				cascade[1]->displacementTexture->getAllSRVIndex(),
				cascade[2]->displacementTexture->getAllSRVIndex() }, co);
				});

			context->setDSConstantBuffer(*renderParamBuffer);

			SETCONSTS({
			context->setPSConstants({
				cascade[0]->displacementTexture->getAllSRVIndex(),
				cascade[0]->derivativeTexture->getAllSRVIndex(),
				cascade[0]->jacobianTexture->getAllSRVIndex(),
				cascade[1]->displacementTexture->getAllSRVIndex(),
				cascade[1]->derivativeTexture->getAllSRVIndex(),
				cascade[1]->jacobianTexture->getAllSRVIndex(),
				cascade[2]->displacementTexture->getAllSRVIndex(),
				cascade[2]->derivativeTexture->getAllSRVIndex(),
				cascade[2]->jacobianTexture->getAllSRVIndex(),
				enviromentCube->getAllSRVIndex() }, co);
				});

			context->setPSConstantBuffer(*renderParamBuffer);

			context->clearDepthStencil(CLEARFLAG::DEPTH, 1.0f, 0);

			context->drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		}
	}

	bool getMinMaxMatrix(DirectX::XMMATRIX* minMaxMatrix)
	{
		DirectX::XMVECTOR corners[8] = {
			{-1.f,-1.f,0.f,1.f},
			{-1.f,1.f,0.f,1.f},
			{1.f,1.f,0.f,1.f},
			{1.f,-1.f,0.f,1.f},
			{-1.f,-1.f,1.f,1.f},
			{-1.f,1.f,1.f,1.f},
			{1.f,1.f,1.f,1.f},
			{1.f,-1.f,1.f,1.f},
		};

		const uint32_t edges[12][2] = {
			{0,1},{1,2},{2,3},{3,0},
			{2,6},{3,7},{1,5},{0,4},
			{4,5},{5,6},{6,7},{7,4}
		};

		{
			const DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, MainCamera::getView() * MainCamera::getProj());

			//transform corner point to world space
			for (uint32_t i = 0; i < 8; i++)
			{
				corners[i] = DirectX::XMVector3TransformCoord(corners[i], invViewProj);
			}
		}

		std::vector<DirectX::XMVECTOR> intersections;

		//get all intersections

		const float bound = 25.f;

		{
			const float a0 = bound;

			for (uint32_t i = 0; i < 12; i++)
			{
				const DirectX::XMVECTOR p0 = corners[edges[i][0]];

				const DirectX::XMVECTOR p1 = corners[edges[i][1]];

				const float y0 = DirectX::XMVectorGetY(p0);

				const float y1 = DirectX::XMVectorGetY(p1);

				if ((y0 - a0) * (y1 - a0) < 0.f)
				{
					const float t = (a0 - y0) / (y1 - y0);

					const DirectX::XMVECTOR intersection = DirectX::XMVectorLerp(p0, p1, t);

					intersections.push_back(intersection);
				}
			}
		}

		{
			const float a0 = -bound;

			for (uint32_t i = 0; i < 12; i++)
			{
				const DirectX::XMVECTOR p0 = corners[edges[i][0]];

				const DirectX::XMVECTOR p1 = corners[edges[i][1]];

				const float y0 = DirectX::XMVectorGetY(p0);

				const float y1 = DirectX::XMVectorGetY(p1);

				if ((y0 - a0) * (y1 - a0) < 0.f)
				{
					const float t = (a0 - y0) / (y1 - y0);

					const DirectX::XMVECTOR intersection = DirectX::XMVectorLerp(p0, p1, t);

					intersections.push_back(intersection);
				}
			}
		}

		//add interior point to intersections too
		for (uint32_t i = 0; i < 8; i++)
		{
			const DirectX::XMVECTOR p0 = corners[i];

			const float y0 = DirectX::XMVectorGetY(p0);

			if (y0<bound && y0>-bound)
			{
				intersections.push_back(p0);
			}
		}

		if (intersections.empty())
		{
			return false;
		}

		//project all points to y=0 plane
		for (uint32_t i = 0; i < intersections.size(); i++)
		{
			intersections[i] = DirectX::XMVectorSetY(intersections[i], 0.f);
		}

		DirectX::XMVECTOR aimPoint0;

		DirectX::XMVECTOR aimPoint1;

		{
			DirectX::XMVECTOR forward = camera->getLookDir();

			if (DirectX::XMVectorGetY(forward) > 0.f)
			{
				forward = DirectX::XMVectorSetY(forward, -DirectX::XMVectorGetY(forward));
			}

			{
				const DirectX::XMVECTOR p0 = camera->getEyePos();

				const DirectX::XMVECTOR p1 = DirectX::XMVectorAdd(p0, forward);

				const float y0 = DirectX::XMVectorGetY(p0);

				const float y1 = DirectX::XMVectorGetY(p1);

				const float t = -y0 / (y1 - y0);

				aimPoint0 = DirectX::XMVectorLerp(p0, p1, t);
			}
		}

		const float af = fabsf(DirectX::XMVectorGetY(camera->getLookDir()));

		aimPoint1 = DirectX::XMVectorAdd(camera->getEyePos(), DirectX::XMVectorScale(camera->getLookDir(), 10.f));

		aimPoint1 = DirectX::XMVectorSetY(aimPoint1, 0.f);

		aimPoint0 = DirectX::XMVectorLerp(aimPoint1, aimPoint0, af);

		const DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(camera->getEyePos(), aimPoint0, camera->getUpVector());

		{
			const DirectX::XMMATRIX viewProj = viewMatrix * MainCamera::getProj();

			for (uint32_t i = 0; i < intersections.size(); i++)
			{
				intersections[i] = DirectX::XMVector3TransformCoord(intersections[i], viewProj);
			}
		}

		float x_min = DirectX::XMVectorGetX(intersections[0]);

		float x_max = DirectX::XMVectorGetX(intersections[0]);

		float y_min = DirectX::XMVectorGetY(intersections[0]);

		float y_max = DirectX::XMVectorGetY(intersections[0]);

		for (uint32_t i = 1; i < intersections.size(); i++)
		{
			const float x = DirectX::XMVectorGetX(intersections[i]);

			const float y = DirectX::XMVectorGetY(intersections[i]);

			if (x > x_max) x_max = x;
			if (x < x_min) x_min = x;
			if (y > y_max) y_max = y;
			if (y < y_min) y_min = y;
		}

		DirectX::XMMATRIX pack(x_max - x_min, 0, 0, x_min,
			0, y_max - y_min, 0, y_min,
			0, 0, 1, 0,
			0, 0, 0, 1);

		pack = DirectX::XMMatrixTranspose(pack);

		*minMaxMatrix = pack * DirectX::XMMatrixInverse(nullptr, viewMatrix * MainCamera::getProj());

		return true;
	}

	DirectX::XMFLOAT4 getWorldPos(const float u, const float v, const DirectX::XMMATRIX& m)
	{
		DirectX::XMVECTOR origin = { u,v,0.f,1.f };

		DirectX::XMVECTOR direction = { u,v,1.f,1.f };

		origin = DirectX::XMVector4Transform(origin, m);

		direction = DirectX::XMVector4Transform(direction, m);

		direction = DirectX::XMVectorSubtract(direction, origin);

		const float l = -DirectX::XMVectorGetY(origin) / DirectX::XMVectorGetY(direction);

		const DirectX::XMVECTOR worldPos = DirectX::XMVectorAdd(origin, DirectX::XMVectorScale(direction, l));

		DirectX::XMFLOAT4 flt;

		DirectX::XMStoreFloat4(&flt, worldPos);

		return flt;
	}

	void updateVertices()
	{
		DirectX::XMMATRIX range;

		if (getMinMaxMatrix(&range))
		{
			renderOcean = true;
		}
		else
		{
			renderOcean = false;

			return;
		}

		//left bottom
		const DirectX::XMFLOAT4 corners0 = getWorldPos(0.f, 0.f, range);

		//right bottom
		const DirectX::XMFLOAT4 corners1 = getWorldPos(1.f, 0.f, range);

		//left top
		const DirectX::XMFLOAT4 corners2 = getWorldPos(0.f, 1.f, range);

		//right top
		const DirectX::XMFLOAT4 corners3 = getWorldPos(1.f, 1.f, range);

		for (uint32_t y = 0; y < gridSize + 1; y++)
		{
			for (uint32_t x = 0; x < gridSize + 1; x++)
			{
				const float u = (float)x / (float)gridSize;

				const float v = (float)y / (float)gridSize;

				DirectX::XMFLOAT4 result;

				result.x = (1.f - v) * ((1.f - u) * corners0.x + u * corners1.x) + v * ((1.f - u) * corners2.x + u * corners3.x);
				result.z = (1.f - v) * ((1.f - u) * corners0.z + u * corners1.z) + v * ((1.f - u) * corners2.z + u * corners3.z);
				result.w = (1.f - v) * ((1.f - u) * corners0.w + u * corners1.w) + v * ((1.f - u) * corners2.w + u * corners3.w);

				const float divide = 1.f / result.w;

				result.x *= divide;

				result.z *= divide;

				const uint32_t writeCoord = y * (gridSize + 1) + x;

				vertices[writeCoord] = { result.x,0.f,result.z };
			}
		}

		context->updateBuffer(*vertexBuffer, vertices.data(), static_cast<uint32_t>(sizeof(DirectX::XMFLOAT3) * vertices.size()));
	}

	struct SpectrumParam
	{
		const uint32_t mapResolution = textureResolution;
		float mapLength;
		const DirectX::XMFLOAT2 wind = { 12.f * cosf(0.93f + Math::halfPi),12.f * sinf(0.93f + Math::halfPi) };
		const float amplitude = 0.000002f;
		const float gravity = 9.81f;
		float cutoffLow;
		float cutoffHigh;
		const DirectX::XMFLOAT4 padding1[14] = {};
	} spectrumParam[cascadeNum];

	struct RenderParam
	{
		float lodScale = 3.f;
		float lengthScale0 = SceneRenderTask::lengthScale0;
		float lengthScale1 = SceneRenderTask::lengthScale1;
		float lengthScale2 = SceneRenderTask::lengthScale2;
		float sunStrength = 0.548f;
		float sunTheta = 0.143f;
		float specularPower = 256.f;
		float foamBias = 1.6f;
		float foamScale = 1.f / 1.6f;
		float sunDirection = 0.93f;
		float exposure;
		float gamma;
		DirectX::XMFLOAT4 padding1[13] = {};
	} renderParam;

	BufferViewPtr vertexBuffer;

	BufferViewPtr indexBuffer;

	ShaderPtr textureCubePS;

	GraphicsStatePtr textureCubeState;

	ShaderPtr gridDebugVS;

	ShaderPtr gridDebugPS;

	GraphicsStatePtr gridDebugState;

	ShaderPtr oceanVS;

	ShaderPtr oceanHS;

	ShaderPtr oceanDS;

	ShaderPtr oceanPS;

	GraphicsStatePtr oceanState;

	ComputeStatePtr spectrumState;

	ComputeStatePtr conjugateState;

	ComputeStatePtr displacementSpectrumState;

	ComputeStatePtr ifftState;

	//sign correction
	ComputeStatePtr permutationState;

	ComputeStatePtr waveMergeState;

	RenderTextureViewPtr enviromentCube;

	//4 channel random gaussian distribution texture
	//mean 0 standard deviation 1
	RenderTextureViewPtr randomGaussTexture;

	//(tildeh0(k))
	//x y
	RenderTextureViewPtr tildeh0Texture;

	//intermediate texture for ifft compute
	RenderTextureViewPtr tempTexture;

	RenderTextureViewPtr originTexture;

	DepthTextureViewPtr depthTexture;

	DynamicCBufferPtr renderParamBuffer;

	DefaultCBufferPtr spectrumParamBuffer[cascadeNum];

	UniquePtr<WaveCascade> cascade[cascadeNum];

	FPSCamera* camera;

	std::vector<DirectX::XMFLOAT3> vertices;

	std::vector<uint32_t> indices;

	bool renderOcean;
};
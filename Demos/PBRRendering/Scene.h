#pragma once

#include"Model.h"

class Scene
{
public:

	PipelineStatePtr pipelineState;

	ShaderPtr vertexShader;

	ShaderPtr pixelShader;

	struct SceneInfo
	{
		DirectX::XMVECTOR lightPos;
		DirectX::XMFLOAT4 lightColor = { 5.f,5.f,5.f,1.f };
		float metallic = 0.5f;
		float roughness = 0.5f;
		uint32_t enableSpecularIBL = 1;
		uint32_t enableDiffuseIBL = 1;
	}sceneInfo;

	Scene(const std::string& filePath, ResourceManager& resManager, const DXGI_FORMAT rtvFormat) :
		vertexShader(Shader::create(File::getRootFolder() + L"PBRVS.cso")),
		pixelShader(Shader::create(File::getRootFolder() + L"PBRPS.cso"))
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_GenSmoothNormals);

		for (uint32_t i = 0; i < scene->mNumMeshes; i++)
		{
			models.push_back(makeUnique<Model>(scene, scene->mMeshes[i], resManager));
		}

		pipelineState = PipelineStateBuilder()
			.setInputElements({
				{"POSITION",0,FMT::RGB32F,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
				{"NORMAL",0,FMT::RGB32F,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
				})
			.setBlendState(PipelineStateHelper::blendReplace)
			.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setRTVFormats({ rtvFormat })
			.setDSVFormat(FMT::D32F)
			.setVS(*vertexShader)
			.setPS(*pixelShader)
			.build();
	}

	~Scene()
	{
	}

	void draw(GraphicsContext* const context, RenderTextureView& prefilterTexture, RenderTextureView& brdfLUTTexture, RenderTextureView& irradianceTexture)
	{
		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setPipelineState(*pipelineState);

		/*
		cbuffer SceneInfo : register(PER_INVOKE_CONSTANTS)
		{
			float4 lightPos;
			float4 lightColor;
			float metallic;
			float roughness;
		}
		*/

		DirectX::XMVECTOR lightPos = MainCamera::getEyePos();

		lightPos = DirectX::XMVectorScale(DirectX::XMVector3Normalize(lightPos), 1.0f);

		sceneInfo.lightPos = lightPos;

		SETCONSTS({
		context->setPSConstants(sceneInfo, co);

		context->setPSConstants({ prefilterTexture.getAllSRVIndex(),brdfLUTTexture.getAllSRVIndex(),irradianceTexture.getAllSRVIndex() }, co);
			});

		for (uint32_t i = 0; i < models.size(); i++)
		{
			models[i]->draw(context);
		}
	}

private:

	std::vector<UniquePtr<Model>> models;

};
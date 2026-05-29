#pragma once

#include"Model.h"

class Scene
{
public:

	PipelineState* pipelineState;

	Shader* vertexShader;

	Shader* pixelShader;

	struct SceneInfo
	{
		DirectX::XMVECTOR lightPos;
		DirectX::XMFLOAT4 lightColor = { 5.f,5.f,5.f,1.f };
		float metallic = 0.5f;
		float roughness = 0.5f;
		uint32_t enableSpecularIBL = 1;
		uint32_t enableDiffuseIBL = 1;
	}sceneInfo;

	Scene(const std::string& filePath, ResourceManager* const resManager, const DXGI_FORMAT rtvFormat) :
		vertexShader(new Shader(Utils::File::getRootFolder() + L"PBRVS.cso")),
		pixelShader(new Shader(Utils::File::getRootFolder() + L"PBRPS.cso"))
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_GenSmoothNormals);

		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			models.push_back(new Model(scene, scene->mMeshes[i], resManager));
		}

		pipelineState = PipelineStateBuilder()
			.setInputElements({
				{"POSITION",0,FMT::RGB32F,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
				{"NORMAL",0,FMT::RGB32F,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
				})
			.setBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT))
			.setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
			.setRasterizerState(PipelineStateHelper::rasterCullBack)
			.setDepthStencilState(PipelineStateHelper::depthCompareLess)
			.setRTVFormats({ rtvFormat })
			.setDSVFormat(FMT::D32F)
			.setVS(vertexShader)
			.setPS(pixelShader)
			.build();
	}

	~Scene()
	{
		if (vertexShader)
			delete vertexShader;

		if (pixelShader)
			delete pixelShader;

		if (pipelineState)
			delete pipelineState;

		for (uint32_t i = 0; i < models.size(); i++)
		{
			if (models[i])
			{
				delete models[i];
			}
		}
	}

	void draw(GraphicsContext* const context, TextureRenderView* const prefilterTexture, TextureRenderView* const brdfLUTTexture, TextureRenderView* const irradianceTexture)
	{
		context->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->setPipelineState(pipelineState);

		/*
		cbuffer SceneInfo : register(b2)
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

		context->setPSConstants(12, &sceneInfo, 0);

		context->setPSConstants({ prefilterTexture->getAllSRVIndex(),brdfLUTTexture->getAllSRVIndex(),irradianceTexture->getAllSRVIndex() }, 12);

		for (uint32_t i = 0; i < models.size(); i++)
		{
			models[i]->draw(context);
		}
	}

private:

	std::vector<Model*> models;

};
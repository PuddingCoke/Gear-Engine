#pragma once

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include<Gear/DevEssential.h>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
};

class Model
{
public:

	const uint32_t vertexNum;

	Model(const aiScene* const scene, const aiMesh* const mesh, ResourceManager& resManager) :
		vertexNum(mesh->mNumVertices)
	{
		{
			std::vector<Vertex> vertices;

			for (uint32_t i = 0; i < vertexNum; i++)
			{
				vertices.push_back(Vertex{ {mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z},{mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z} });
			}

			vertexBuffer = resManager.createStructuredBufferView(sizeof(Vertex), sizeof(Vertex) * vertexNum,
				false, false, true, false, true, vertices.data());
		}

		{
			struct MaterialProperty
			{
				DirectX::XMFLOAT4 ambientColor;
				DirectX::XMFLOAT4 diffuseColor;
				DirectX::XMFLOAT4 specularColor;
				DirectX::XMFLOAT4 padding[13];
			}property;

			aiColor3D color;

			scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_AMBIENT, color);

			property.ambientColor = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.f);

			scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

			property.diffuseColor = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.f);

			scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_SPECULAR, color);

			property.specularColor = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.f);

			materialBuffer = resManager.createImmutableCBuffer(sizeof(MaterialProperty), &property, true);
		}
	}

	~Model()
	{
	}

	void draw(GraphicsContext* const context) const
	{
		context->setPSConstantBuffer(*materialBuffer);

		context->setVertexBuffers({ *vertexBuffer }, 0);

		context->draw(vertexNum, 1, 0, 0);
	}

private:

	BufferViewPtr vertexBuffer;

	ImmutableCBufferPtr materialBuffer;

};
#pragma once

const std::string assetPath = "E:/glTF-Sample-Assets/Models/Sponza/glTF/";

const std::wstring wAssetPath = L"E:/glTF-Sample-Assets/Models/Sponza/glTF/";

#include"Model.h"
#include"Material.h"

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

class Scene
{
public:

	Scene(const std::string& filePath, ResourceManager& resManager)
	{
		Assimp::Importer importer;

		const aiScene* const scene = importer.ReadFile(filePath, aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_ConvertToLeftHanded);

		for (uint32_t materialIdx = 0; materialIdx < scene->mNumMaterials; materialIdx++)
		{
			const aiMaterial* const material = scene->mMaterials[materialIdx];

			aiString texturePath;
			std::string diffusePath;
			std::string roughnessMetallicPathPath;
			std::string normalPath;

			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			{
				material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
				diffusePath = assetPath + texturePath.C_Str();
			}
			else
			{
				diffusePath = assetPath + "dummy.dds";
			}

			if (material->GetTextureCount(aiTextureType_METALNESS) > 0)
			{
				material->GetTexture(aiTextureType_METALNESS, 0, &texturePath);
				roughnessMetallicPathPath = assetPath + texturePath.C_Str();
			}
			else
			{
				roughnessMetallicPathPath = assetPath + "dummy_specular.dds";
			}

			if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
			{
				material->GetTexture(aiTextureType_NORMALS, 0, &texturePath);
				normalPath = assetPath + texturePath.C_Str();
			}
			else
			{
				normalPath = assetPath + "dummy_ddn.dds";
			}

			materials.push_back(makeUnique<Material>(resManager, diffusePath, roughnessMetallicPathPath, normalPath));
		}

		std::vector<Vertex> vertices;

		std::vector<uint32_t> indices;

		uint32_t startVertexLocation = 0;

		uint32_t startIndexLocation = 0;

		for (uint32_t meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++)
		{
			const aiMesh* const mesh = scene->mMeshes[meshIdx];

			const bool hasUV = mesh->HasTextureCoords(0);

			const bool hasTangent = mesh->HasTangentsAndBitangents();

			const uint32_t vertexCount = mesh->mNumVertices;

			for (uint32_t vertIdx = 0; vertIdx < mesh->mNumVertices; vertIdx++)
			{
				Vertex vert = {};

				{
					const aiVector3D& position = mesh->mVertices[vertIdx];

					vert.pos = DirectX::XMFLOAT3(position.x, position.y, position.z);

					//缩放、平移一下
					//导入到Blender中发现和Crytek Sponza有点区别
					vert.pos.x *= 12.5f;
					vert.pos.y *= 12.5f;
					vert.pos.z *= 12.5f;

					vert.pos.x += 12.74f;

					vert.pos.z -= 3.61f;
				}

				{
					const aiVector3D& normal = mesh->mNormals[vertIdx];

					vert.normal = DirectX::XMFLOAT3(normal.x, normal.y, normal.z);
				}

				if (hasUV)
				{
					const aiVector3D& uv = mesh->mTextureCoords[0][vertIdx];

					vert.uv = DirectX::XMFLOAT2(uv.x, uv.y);
				}
				else
				{
					vert.uv = DirectX::XMFLOAT2(0.f, 0.f);
				}

				if (hasTangent)
				{
					const aiVector3D& tangent = mesh->mTangents[vertIdx];

					const aiVector3D& binormal = mesh->mBitangents[vertIdx];

					vert.tangent = DirectX::XMFLOAT3(tangent.x, tangent.y, tangent.z);
					vert.binormal = DirectX::XMFLOAT3(binormal.x, binormal.y, binormal.z);
				}
				else
				{
					vert.tangent = DirectX::XMFLOAT3(0.f, 1.f, 0.f);
					vert.binormal = DirectX::XMFLOAT3(0.f, 1.f, 0.f);
				}

				vertices.push_back(vert);
			}

			const uint32_t indexCount = mesh->mNumFaces * 3;

			for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
			{
				const aiFace& face = mesh->mFaces[faceIndex];

				for (uint32_t index = 0; index < face.mNumIndices; index++)
				{
					indices.push_back(face.mIndices[index]);
				}
			}

			models.push_back(makeUnique<Model>(mesh->mMaterialIndex, indexCount, startIndexLocation, startVertexLocation));

			startVertexLocation += vertexCount;

			startIndexLocation += indexCount;
		}

		modelBuffer = resManager.createStructuredBufferView(sizeof(Vertex), sizeof(Vertex) * vertices.size(), true, false, true, false, true, vertices.data());

		indexBuffer = resManager.createTypedBufferView(FMT::R32UI, sizeof(uint32_t) * indices.size(), false, false, false, true, false, true, indices.data());
	}

	~Scene()
	{
	}

	void render(GraphicsContext* const context)
	{
		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);
		context->setVertexBuffers({ modelBuffer->getVertexBuffer() }, 0);
		context->setIndexBuffer(indexBuffer->getIndexBuffer());

		for (uint32_t i = 0; i < models.size(); i++)
		{
			materials[models[i]->materialIndex]->bind(context);
			models[i]->draw(context);
		}
	}

	//rasterize whole scene to a texture cube
	void renderCube(GraphicsContext* const context)
	{
		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);
		context->setVertexBuffers({ modelBuffer->getVertexBuffer() }, 0);
		context->setIndexBuffer(indexBuffer->getIndexBuffer());

		for (uint32_t i = 0; i < models.size(); i++)
		{
			materials[models[i]->materialIndex]->bind(context);
			models[i]->drawCube(context);
		}
	}

private:

	std::vector<UniquePtr<Material>> materials;

	std::vector<UniquePtr<Model>> models;

	BufferViewPtr modelBuffer;

	BufferViewPtr indexBuffer;

};
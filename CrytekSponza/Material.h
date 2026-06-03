#pragma once

#include<Gear/Core/GraphicsContext.h>
#include<Gear/Core/ResourceManager.h>

class Material
{
public:

	Material(ResourceManager& resManager, const std::string& diffusePath, const std::string& roughnessMetallicPath, std::string& normalPath)
	{
		const std::wstring wDiffusePath = std::wstring(diffusePath.begin(), diffusePath.end());

		const std::wstring wRoughnessMetallicPath = std::wstring(roughnessMetallicPath.begin(), roughnessMetallicPath.end());

		const std::wstring wNormalPath = std::wstring(normalPath.begin(), normalPath.end());

		diffuse = resManager.createTextureRenderView(wDiffusePath, true);

		roughnessMetallic = resManager.createTextureRenderView(wRoughnessMetallicPath, true);

		normal = resManager.createTextureRenderView(wNormalPath, true);
	}

	~Material()
	{
	}

	//use first 3 slots of pixel constants
	void bind(GraphicsContext* const context)
	{
		context->setPSConstants({ diffuse->getAllSRVIndex(),roughnessMetallic->getAllSRVIndex(),normal->getAllSRVIndex() }, 0);
	}

private:

	UniquePtr<TextureRenderView> diffuse;

	UniquePtr<TextureRenderView> roughnessMetallic;

	UniquePtr<TextureRenderView> normal;

};
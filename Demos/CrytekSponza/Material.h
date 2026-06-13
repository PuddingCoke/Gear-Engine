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

		diffuse = resManager.createRenderTextureView(wDiffusePath, true);

		roughnessMetallic = resManager.createRenderTextureView(wRoughnessMetallicPath, true);

		normal = resManager.createRenderTextureView(wNormalPath, true);
	}

	~Material()
	{
	}

	//use first 3 slots of pixel constants
	void bind(GraphicsContext* const context)
	{
		SETCONSTS({
		context->setPSConstants({ diffuse->getAllSRVIndex(),roughnessMetallic->getAllSRVIndex(),normal->getAllSRVIndex() }, co);
			});
	}

private:

	RenderTextureViewPtr diffuse;

	RenderTextureViewPtr roughnessMetallic;

	RenderTextureViewPtr normal;

};
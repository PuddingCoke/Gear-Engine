#pragma once

#include<Gear/Core/GraphicsContext.h>
#include<Gear/Core/ResourceManager.h>

class Material
{
public:

	Material(ResourceManager& resManager, const std::string& diffusePath, const std::string& roughnessMetallicPath, std::string& normalPath)
	{
		diffuse = resManager.createRenderTextureView(String::stringToWString(diffusePath), true);

		roughnessMetallic = resManager.createRenderTextureView(String::stringToWString(roughnessMetallicPath), true);

		normal = resManager.createRenderTextureView(String::stringToWString(normalPath), true);
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

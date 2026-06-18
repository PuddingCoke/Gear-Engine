#pragma once

#include<Gear/Core/GraphicsContext.h>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 binormal;
};

class Model
{
public:

	Model(const uint32_t materialIndex, const uint32_t indexCount, const uint32_t startIndexLocation, const uint32_t startVertexLocation) :
		materialIndex(materialIndex),
		indexCount(indexCount),
		startIndexLocation(startIndexLocation),
		startVertexLocation(startVertexLocation)
	{

	}

	void draw(GraphicsContext* const context) const
	{
		context->drawIndexed(indexCount, 1, startIndexLocation, startVertexLocation, 0);
	}

	void drawCube(GraphicsContext* const context) const
	{
		context->drawIndexed(indexCount, 6, startIndexLocation, startVertexLocation, 0);
	}

	const uint32_t materialIndex;

	const uint32_t indexCount;

	const uint32_t startIndexLocation;

	const uint32_t startVertexLocation;

};
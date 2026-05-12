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

constexpr D3D12_INPUT_ELEMENT_DESC inputDesc[5] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};

class Model
{
public:

	Model(const UINT materialIndex, const UINT indexCount, const UINT startIndexLocation, const UINT startVertexLocation) :
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

	const UINT materialIndex;

	const UINT indexCount;

	const UINT startIndexLocation;

	const UINT startVertexLocation;

};
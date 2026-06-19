#pragma once

#ifndef _GEAR_CORE_D3D12RESOURCE_PIPELINERESOURCEDESC_H_
#define _GEAR_CORE_D3D12RESOURCE_PIPELINERESOURCEDESC_H_

#include<D3D12Headers/d3dx12.h>

namespace Gear::Core::D3D12Resource
{
	class Buffer;

	class Texture;

	struct ShaderResourceDesc
	{
		enum ResourceType
		{
			BUFFER,
			TEXTURE
		} type;

		enum TargetStates
		{
			SRV,
			UAV,
			CBV
		} state;

		uint32_t resourceIndex;

		struct TextureTransitionDesc
		{
			Texture* texture;
			uint32_t mipSlice;
		};

		struct BufferTransitionDesc
		{
			Buffer* buffer;
			Buffer* counterBuffer;
		};

		union
		{
			TextureTransitionDesc textureDesc;
			BufferTransitionDesc bufferDesc;
		};
	};

	struct RenderTargetDesc
	{
		Texture* texture;
		uint32_t mipSlice;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		DXGI_FORMAT rtvFormat;
	};

	struct DepthStencilDesc
	{
		Texture* texture;
		uint32_t mipSlice;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
		DXGI_FORMAT dsvFormat;
	};

	struct VertexBufferDesc
	{
		Buffer* buffer;
		D3D12_VERTEX_BUFFER_VIEW vbv;
	};

	struct IndexBufferDesc
	{
		Buffer* buffer;
		D3D12_INDEX_BUFFER_VIEW ibv;
	};

	struct ClearUAVDesc
	{
		enum ResourceType
		{
			BUFFER,
			TEXTURE
		} type;

		struct TextureClearDesc
		{
			Texture* texture;
			uint32_t mipSlice;
		};

		struct BufferClearDesc
		{
			Buffer* buffer;
		};

		union
		{
			TextureClearDesc textureDesc;
			BufferClearDesc bufferDesc;
		};

		D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle;
	};
}

#endif // !_GEAR_CORE_D3D12RESOURCE_PIPELINERESOURCEDESC_H_

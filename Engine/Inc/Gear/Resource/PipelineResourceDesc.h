#pragma once

#ifndef _GEAR_RESOURCE_PIPELINERESOURCEDESC_H_
#define _GEAR_RESOURCE_PIPELINERESOURCEDESC_H_

#include<D3D12Headers/d3dx12.h>

namespace Gear::Core::D3D12Resource
{
	class Buffer;

	class Texture;
}

namespace Gear::Resource
{
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

		const uint32_t* resourceIndex;

		struct TextureTransitionDesc
		{
			Core::D3D12Resource::Texture* texture;
			uint32_t mipSlice;
		};

		struct BufferTransitionDesc
		{
			Core::D3D12Resource::Buffer* buffer;
			Core::D3D12Resource::Buffer* counterBuffer;
		};

		union
		{
			TextureTransitionDesc textureDesc;
			BufferTransitionDesc bufferDesc;
		};
	};

	struct RenderTargetDesc
	{
		Core::D3D12Resource::Texture* texture;
		uint32_t mipSlice;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		DXGI_FORMAT rtvFormat;
	};

	struct DepthStencilDesc
	{
		Core::D3D12Resource::Texture* texture;
		uint32_t mipSlice;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
		DXGI_FORMAT dsvFormat;
	};

	struct VertexBufferDesc
	{
		Core::D3D12Resource::Buffer* buffer;
		D3D12_VERTEX_BUFFER_VIEW vbv;
	};

	struct IndexBufferDesc
	{
		Core::D3D12Resource::Buffer* buffer;
		D3D12_INDEX_BUFFER_VIEW ibv;
	};

	struct UAVClearDesc
	{
		enum ResourceType
		{
			BUFFER,
			TEXTURE
		} type;

		struct TextureClearDesc
		{
			Core::D3D12Resource::Texture* texture;
			uint32_t mipSlice;
		};

		struct BufferClearDesc
		{
			Core::D3D12Resource::Buffer* buffer;
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

#endif // !_GEAR_RESOURCE_PIPELINERESOURCEDESC_H_

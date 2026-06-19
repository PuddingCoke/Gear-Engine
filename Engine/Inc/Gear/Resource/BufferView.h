#pragma once

#ifndef _GEAR_RESOURCE_BUFFERVIEW_H_
#define _GEAR_RESOURCE_BUFFERVIEW_H_

#include<Gear/Core/D3D12Resource/UploadHeap.h>

#include"CounterBufferView.h"

namespace Gear::Resource
{
	CREATESAFETYPE(BufferView);

	//多用途缓冲
	class BufferView :public ResourceBase
	{
	public:

		BufferView() = delete;

		BufferView(D3D12Resource::BufferPtr bufferPtr, const uint32_t structureByteStride, const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent);

		BufferView(const BufferView&);

		~BufferView();

		D3D12Resource::VertexBufferDesc getVertexBuffer() const;

		D3D12Resource::IndexBufferDesc getIndexBuffer() const;

		D3D12Resource::ShaderResourceDesc getSRVIndex() const;

		D3D12Resource::ShaderResourceDesc getUAVIndex() const;

		D3D12Resource::ClearUAVDesc getClearUAVDesc() const;

		CounterBufferView* getCounterBuffer() const;

		D3D12Resource::Buffer* getBuffer() const;

		void copyDescriptors() override;

		struct UpdateStruct
		{
			D3D12Resource::Buffer* const buffer;
			D3D12Resource::UploadHeap* const uploadHeap;
		};

		UpdateStruct getUpdateStruct(const void* const data, const uint64_t size);

	private:

		const bool hasSRV;

		const bool hasUAV;

		DXGI_FORMAT srvFormat;

		DXGI_FORMAT uavFormat;

		CounterBufferViewPtr counterBuffer;

		SharedPtr<uint32_t> srvIndex;

		SharedPtr<uint32_t> uavIndex;

		SharedPtr<D3D12_GPU_DESCRIPTOR_HANDLE> viewGPUHandle;

		SharedPtr<D3D12_CPU_DESCRIPTOR_HANDLE> viewCPUHandle;

		union BufferViewStruct
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;

			D3D12_INDEX_BUFFER_VIEW ibv;
		};

		SharedPtr<BufferViewStruct> bufferViewStruct;

		UniquePtr<UniquePtr<D3D12Resource::UploadHeap>[]> uploadHeaps;

		D3D12Resource::BufferPtr buffer;
	};
}

#endif // !_GEAR_RESOURCE_BUFFERVIEW_H_
#pragma once

#ifndef _GEAR_RESOURCE_BUFFERVIEW_H_
#define _GEAR_RESOURCE_BUFFERVIEW_H_

#include <Gear/Core/D3D12Resource/UploadHeap.h>

#include "CounterBufferView.h"

namespace Gear::Resource
{
	CREATESAFETYPE(BufferView);

	//多用途缓冲
	class BufferView : public ResourceBase
	{
	public:

		BufferView() = delete;

		BufferView(D3D12Resource::BufferPtr bufferPtr,
		           const uint32_t structureByteStride,
		           const DXGI_FORMAT format,
		           const bool createSRV,
		           const bool createUAV,
		           const bool createVBV,
		           const bool createIBV,
		           const bool cpuWritable);

		BufferView(const BufferView&);

		~BufferView();

		VertexBufferDesc getVertexBuffer() const;

		IndexBufferDesc getIndexBuffer() const;

		ShaderResourceDesc getSRVIndex();

		ShaderResourceDesc getUAVIndex();

		UAVClearDesc getUAVClearDesc();

		CounterBufferView* getCounterBuffer() const;

		D3D12Resource::Buffer* getBuffer() const;

		struct UpdateStruct
		{
			D3D12Resource::Buffer* const buffer;
			D3D12Resource::UploadHeap* const uploadHeap;
		};

		UpdateStruct getUpdateStruct(const void* const data, const uint64_t size);

		operator VertexBufferDesc() const;

		operator IndexBufferDesc() const;

	private:

		const bool hasSRV;

		const bool hasUAV;

		DXGI_FORMAT srvFormat;

		DXGI_FORMAT uavFormat;

		uint32_t srvIndex;

		uint32_t uavIndex;

		D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle;

		union BufferViewUnion
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;

			D3D12_INDEX_BUFFER_VIEW ibv;
		};

		BufferViewUnion bufferViewUnion;

		UniquePtr<D3D12Resource::UploadHeapPtr[]> uploadHeaps;

		D3D12Resource::BufferPtr buffer;

		CounterBufferViewPtr counterBuffer;
	};
}

#endif // !_GEAR_RESOURCE_BUFFERVIEW_H_

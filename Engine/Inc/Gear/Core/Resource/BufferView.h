#pragma once

#ifndef _GEAR_CORE_RESOURCE_BUFFERVIEW_H_
#define _GEAR_CORE_RESOURCE_BUFFERVIEW_H_

#include"D3D12Resource/UploadHeap.h"

#include"CounterBufferView.h"

namespace Gear
{
	namespace Core
	{
		namespace Resource
		{
			//多用途缓冲
			class BufferView :public ResourceBase
			{
			public:

				BufferView() = delete;

				BufferView(D3D12Resource::Buffer* const buffer, const uint32_t structureByteStride, const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent);

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

				const bool hasSRV;

				const bool hasUAV;

			private:

				CounterBufferView* counterBuffer;

				uint32_t srvIndex;

				uint32_t uavIndex;

				D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandle;

				D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle;

				union
				{
					D3D12_VERTEX_BUFFER_VIEW vbv;

					D3D12_INDEX_BUFFER_VIEW ibv;
				};

				D3D12Resource::UploadHeap** uploadHeaps;

				D3D12Resource::Buffer* buffer;
			};
		}
	}
}

#endif // !_GEAR_CORE_RESOURCE_BUFFERVIEW_H_
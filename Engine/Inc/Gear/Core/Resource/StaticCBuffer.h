#pragma once

#ifndef _GEAR_CORE_RESOURCE_STATICCBUFFER_H_
#define _GEAR_CORE_RESOURCE_STATICCBUFFER_H_

#include<Gear/Core/Resource/D3D12Resource/UploadHeap.h>

#include"ImmutableCBuffer.h"

namespace Gear::Core::Resource
{
	class StaticCBuffer :public ImmutableCBuffer
	{
	public:

		StaticCBuffer(UniquePtr<D3D12Resource::Buffer> bufferPtr, const uint32_t size, const bool persistent);

		~StaticCBuffer();

		struct UpdateStruct
		{
			D3D12Resource::Buffer* const buffer;
			D3D12Resource::UploadHeap* const uploadHeap;
		};

		UpdateStruct getUpdateStruct(const void* const data, const uint64_t size);

	private:

		UniquePtr<UniquePtr<D3D12Resource::UploadHeap>[]> uploadHeaps;

		UniquePtr<void* []> dataPtrs;

	};
}

#endif // !_GEAR_CORE_RESOURCE_STATICCBUFFER_H_

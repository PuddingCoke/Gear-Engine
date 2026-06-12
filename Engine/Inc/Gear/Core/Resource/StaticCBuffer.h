#pragma once

#ifndef _GEAR_CORE_RESOURCE_STATICCBUFFER_H_
#define _GEAR_CORE_RESOURCE_STATICCBUFFER_H_

#include<Gear/Core/Resource/D3D12Resource/UploadHeap.h>

#include"ImmutableCBuffer.h"

namespace Gear::Core::Resource
{
	CREATESAFETYPE(StaticCBuffer);

	//注意事项：静态常量缓冲的更新必须置于使用这个常量缓冲的API前
	//而动态常量缓冲可以在任意位置调用更新API，这是因为引擎有相关的管理框架
	class StaticCBuffer :public ImmutableCBuffer
	{
	public:

		StaticCBuffer(D3D12Resource::BufferPtr bufferPtr, const uint32_t size, const bool persistent);

		StaticCBuffer(const StaticCBuffer&);

		~StaticCBuffer();

		struct UpdateStruct
		{
			D3D12Resource::Buffer* const buffer;
			D3D12Resource::UploadHeap* const uploadHeap;
		};

		UpdateStruct getUpdateStruct(const void* const data, const uint64_t size);

	private:

		UniquePtr<UniquePtr<D3D12Resource::UploadHeap>[]> uploadHeaps;

		SharedPtr<void* []> dataPtrs;

	};
}

#endif // !_GEAR_CORE_RESOURCE_STATICCBUFFER_H_

#pragma once

#ifndef _GEAR_RESOURCE_DEFAULTCBUFFER_H_
#define _GEAR_RESOURCE_DEFAULTCBUFFER_H_

#include<Gear/Core/D3D12Resource/UploadHeap.h>

#include"ImmutableCBuffer.h"

namespace Gear::Resource
{
	CREATESAFETYPE(DefaultCBuffer);

	//注意事项：默认常量缓冲的更新必须置于使用这个常量缓冲的API前
	//而对于动态常量缓冲来说，你可以在任意位置调用更新API，这是因为引擎有相关的管理框架
	class DefaultCBuffer :public ImmutableCBuffer
	{
	public:

		DefaultCBuffer(D3D12Resource::BufferPtr bufferPtr, const bool persistent);

		DefaultCBuffer(const DefaultCBuffer&);

		~DefaultCBuffer();

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

#endif // !_GEAR_RESOURCE_DEFAULTCBUFFER_H_

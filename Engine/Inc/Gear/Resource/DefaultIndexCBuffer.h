#pragma once

#ifndef _GEAR_RESOURCE_DEFAULTINDEXCBUFFER_H_
#define _GEAR_RESOURCE_DEFAULTINDEXCBUFFER_H_

#include<Gear/Core/D3D12Resource/UploadHeap.h>

#include<Gear/Core/Graphics.h>

#include"DefaultIndexCBufferBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(DefaultIndexCBuffer);

	class DefaultIndexCBuffer :public DefaultIndexCBufferBase
	{
	public:

		template<size_t N>
		DefaultIndexCBuffer(const ResourceDescPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr);

		~DefaultIndexCBuffer();

		struct UpdateStruct
		{
			D3D12Resource::Buffer* const buffer;
			D3D12Resource::UploadHeap* const uploadHeap;
		};

		bool getNeedUpdate();

		UpdateStruct getUpdateStruct();

	private:

		UniquePtr<UniquePtr<Core::D3D12Resource::UploadHeap>[]> uploadHeaps;

		SharedPtr<void* []> dataPtrs;

	};

	template<size_t N>
	inline DefaultIndexCBuffer::DefaultIndexCBuffer(const ResourceDescPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr) :
		DefaultIndexCBufferBase(pairs, std::move(bufferPtr)),
		uploadHeaps(makeUnique<UniquePtr<D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount())),
		dataPtrs(makeShared<void* []>(Graphics::getFrameBufferCount()))
	{
		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			uploadHeaps[i] = makeUnique<D3D12Resource::UploadHeap>(buffer->getSize());

			dataPtrs[i] = uploadHeaps[i]->map();
		}
	}
}

#endif // !_GEAR_RESOURCE_DEFAULTINDEXCBUFFER_H_

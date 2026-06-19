#pragma once

#ifndef _GEAR_RESOURCE_DEFAULTINDEXCBUFFER_H_
#define _GEAR_RESOURCE_DEFAULTINDEXCBUFFER_H_

#include<Gear/Core/D3D12Resource/UploadHeap.h>

#include"IndexCBufferBase.h"

#include"ImmutableIndexCBuffer.h"

namespace Gear::Resource
{
	class DefaultIndexCBuffer :public ImmutableIndexCBuffer, public IndexCBufferBase
	{
	public:

		template<size_t N>
		DefaultIndexCBuffer(const ResourceIndexPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr);

		template<size_t N>
		void update(const ResourceIndexPair(&pairs)[N]);

	private:

		UniquePtr<UniquePtr<Core::D3D12Resource::UploadHeap>[]> uploadHeaps;

		SharedPtr<void* []> dataPtrs;

	};

	template<size_t N>
	inline DefaultIndexCBuffer::DefaultIndexCBuffer(const ResourceIndexPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr) :
		ImmutableIndexCBuffer(pairs, std::move(buffer)), IndexCBufferBase(pairs),
		uploadHeaps(makeUnique<UniquePtr<D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount())),
		dataPtrs(makeShared<void* []>(Graphics::getFrameBufferCount()))
	{
	}

	template<size_t N>
	inline void DefaultIndexCBuffer::update(const ResourceIndexPair(&pairs)[N])
	{
		updateIndexPairs(pairs);

		updateShaderResourceDescs(pairs);

		updateResourceIndices();
	}
}

#endif // !_GEAR_RESOURCE_DEFAULTINDEXCBUFFER_H_

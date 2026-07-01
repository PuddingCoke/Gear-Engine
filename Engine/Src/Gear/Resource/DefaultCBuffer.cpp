#include<Gear/Resource/DefaultCBuffer.h>

#include<Gear/Core/Graphics.h>

namespace Gear::Resource
{
	DefaultCBuffer::DefaultCBuffer(D3D12Resource::BufferPtr bufferPtr, const bool persistent) :
		ImmutableCBuffer(std::move(bufferPtr), persistent),
		uploadHeaps(makeUnique<UniquePtr<D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount())),
		dataPtrs(makeShared<void* []>(Graphics::getFrameBufferCount()))
	{
		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			uploadHeaps[i] = makeUnique<D3D12Resource::UploadHeap>(buffer->getSize());

			dataPtrs[i] = uploadHeaps[i]->map();
		}
	}

	DefaultCBuffer::DefaultCBuffer(const DefaultCBuffer& scb) :
		ImmutableCBuffer(scb),
		uploadHeaps(nullptr),
		dataPtrs(scb.dataPtrs)
	{
	}

	DefaultCBuffer::~DefaultCBuffer()
	{
		if (uploadHeaps.get())
		{
			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				if (uploadHeaps[i].get())
				{
					uploadHeaps[i]->unmap();
				}
			}
		}
	}

	DefaultCBuffer::UpdateStruct DefaultCBuffer::getUpdateStruct(const void* const data, const uint64_t size)
	{
		memcpy(dataPtrs[Graphics::getFrameIndex()], data, size);

		const UpdateStruct updateStruct = { buffer.get(),uploadHeaps[Graphics::getFrameIndex()].get() };

		return updateStruct;
	}
}

#include <Gear/Resource/DefaultCBuffer.h>

#include <Gear/Core/Graphics.h>

namespace Gear::Resource
{
	DefaultCBuffer::DefaultCBuffer(D3D12Resource::BufferPtr bufferPtr) :
		ImmutableCBuffer(std::move(bufferPtr)),
		uploadHeaps(makeUnique<UniquePtr<D3D12Resource::UploadHeap>[]>(Graphics::getFrameBufferCount())),
		dataPtrs(makeShared<void*[]>(Graphics::getFrameBufferCount()))
	{
		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			uploadHeaps[i] = makeUnique<D3D12Resource::UploadHeap>(buffer->getSize());

			dataPtrs[i] = uploadHeaps[i]->map();
		}
	}

	// TODO 待办 #1：拷贝构造没有重建 uploadHeaps，dataPtrs 还指向源对象的映射内存
	// 拷贝出来的对象调用 getUpdateStruct 会解引用空指针
#pragma message("TODO: DefaultCBuffer 的拷贝构造尚未完成（uploadHeaps 为 nullptr）")
	DefaultCBuffer::DefaultCBuffer(const DefaultCBuffer& scb) :
		ImmutableCBuffer(scb), uploadHeaps(nullptr), dataPtrs(scb.dataPtrs)
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

		const UpdateStruct updateStruct = { buffer.get(), uploadHeaps[Graphics::getFrameIndex()].get() };

		return updateStruct;
	}
}

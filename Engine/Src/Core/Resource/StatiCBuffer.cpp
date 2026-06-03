#include<Gear/Core/Resource/StaticCBuffer.h>

#include<Gear/Core/Graphics.h>

namespace Gear::Core::Resource
{
	StaticCBuffer::StaticCBuffer(D3D12Resource::Buffer* const buffer, const uint32_t size, const bool persistent) :
		ImmutableCBuffer(buffer, size, persistent)
	{
		uploadHeaps = new D3D12Resource::UploadHeap * [Graphics::getFrameBufferCount()];

		dataPtrs = new void* [Graphics::getFrameBufferCount()];

		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			uploadHeaps[i] = new D3D12Resource::UploadHeap(size);

			dataPtrs[i] = uploadHeaps[i]->map();
		}
	}

	StaticCBuffer::~StaticCBuffer()
	{
		if (uploadHeaps)
		{
			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				if (uploadHeaps[i])
				{
					uploadHeaps[i]->unmap();

					delete uploadHeaps[i];
				}
			}

			delete[] uploadHeaps;
		}

		if (dataPtrs)
		{
			delete[] dataPtrs;
		}
	}

	StaticCBuffer::UpdateStruct StaticCBuffer::getUpdateStruct(const void* const data, const uint64_t size)
	{
		memcpy(dataPtrs[Graphics::getFrameIndex()], data, size);

		const UpdateStruct updateStruct = { buffer,uploadHeaps[Graphics::getFrameIndex()] };

		return updateStruct;
	}
}

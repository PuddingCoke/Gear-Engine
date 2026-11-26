#include <Gear/Resource/ImmutableCBuffer.h>

namespace Gear::Resource
{
	ImmutableCBuffer::ImmutableCBuffer(D3D12Resource::BufferPtr bufferPtr) :
		gpuAddress(D3D12_GPU_VIRTUAL_ADDRESS()), bufferIndex(UINT32_MAX), buffer(std::move(bufferPtr))
	{
		if (buffer)
		{
			if (buffer->getSize() % 256ull != 0)
			{
				THROWLOG(LOGERROR() << "常量缓冲的字节大小必须是256的倍数！");
			}

			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = buffer->getGPUAddress();
			desc.SizeInBytes = static_cast<uint32_t>(buffer->getSize());

			D3D12Core::DescriptorHandle* const descriptorHandle = createResourceDescriptorHandle(1);

			GraphicsDevice::get()->CreateConstantBufferView(&desc, descriptorHandle->getCurrentCPUHandle());

			gpuAddress = buffer->getGPUAddress();

			bufferIndex = descriptorHandle->getCurrentIndex();
		}
	}

	ImmutableCBuffer::ImmutableCBuffer(const ImmutableCBuffer& icb) :
		ResourceBase(icb),
		gpuAddress(icb.gpuAddress),
		bufferIndex(icb.bufferIndex),
		buffer(icb.buffer ? makeUnique<D3D12Resource::Buffer>(*icb.buffer) : nullptr)
	{
	}

	ImmutableCBuffer::~ImmutableCBuffer()
	{
	}

	ShaderResourceDesc ImmutableCBuffer::getBufferIndex()
	{
		const ShaderResourceDesc desc = { .type = ShaderResourceDesc::BUFFER,
			                              .state = ShaderResourceDesc::CBV,
			                              .resourceIndex = bufferIndex,
			                              .bufferDesc = { .buffer = buffer.get(),
			                                              .counterBuffer = nullptr } };

		return desc;
	}

	D3D12_GPU_VIRTUAL_ADDRESS ImmutableCBuffer::getGPUAddress() const
	{
		return gpuAddress;
	}

	D3D12Resource::Buffer* ImmutableCBuffer::getBuffer() const
	{
		return buffer.get();
	}
}

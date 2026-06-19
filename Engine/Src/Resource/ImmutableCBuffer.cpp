#include<Gear/Resource/ImmutableCBuffer.h>

namespace Gear::Resource
{
	ImmutableCBuffer::ImmutableCBuffer(D3D12Resource::BufferPtr bufferPtr, const uint32_t size, const bool persistent) :
		ResourceBase(persistent),
		gpuAddress(makeShared<D3D12_GPU_VIRTUAL_ADDRESS>()),
		bufferIndex(makeShared<uint32_t>()),
		buffer(std::move(bufferPtr))
	{
		if (size % 256 != 0)
		{
			LOGERROR(L"常量缓冲的字节大小必须是256的倍数！");
		}

		if (buffer)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = buffer->getGPUAddress();
			desc.SizeInBytes = size;

			setNumCBVSRVUAVDescriptors(1);

			D3D12Core::DescriptorHandle descriptorHandle = allocCBVSRVUAVDescriptors();

			GraphicsDevice::get()->CreateConstantBufferView(&desc, descriptorHandle.getCurrentCPUHandle());

			*gpuAddress = buffer->getGPUAddress();

			*bufferIndex = descriptorHandle.getCurrentIndex();
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

	D3D12Resource::ShaderResourceDesc ImmutableCBuffer::getBufferIndex() const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
		desc.state = D3D12Resource::ShaderResourceDesc::CBV;
		desc.resourceIndex = *bufferIndex;
		desc.bufferDesc.buffer = buffer.get();

		return desc;
	}

	D3D12_GPU_VIRTUAL_ADDRESS ImmutableCBuffer::getGPUAddress() const
	{
		return *gpuAddress;
	}

	D3D12Resource::Buffer* ImmutableCBuffer::getBuffer() const
	{
		return buffer.get();
	}

	void ImmutableCBuffer::copyDescriptors()
	{
		D3D12Core::DescriptorHandle shaderVisibleHandle;

		if (copyToResourceHeap(shaderVisibleHandle))
		{
			*bufferIndex = shaderVisibleHandle.getCurrentIndex();
		}
	}
}
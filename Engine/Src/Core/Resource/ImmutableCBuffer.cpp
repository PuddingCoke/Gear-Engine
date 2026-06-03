#include<Gear/Core/Resource/ImmutableCBuffer.h>

namespace Gear::Core::Resource
{
	ImmutableCBuffer::ImmutableCBuffer(D3D12Resource::Buffer* const buffer, const uint32_t size, const bool persistent) :
		ResourceBase(persistent), gpuAddress(), bufferIndex(), buffer(buffer)
	{
		if (size % 256 != 0)
		{
			LOGERROR(L"size of constant buffer must be multiply of 256");
		}

		if (buffer)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = buffer->getGPUAddress();
			desc.SizeInBytes = size;

			setNumCBVSRVUAVDescriptors(1);

			D3D12Core::DescriptorHandle descriptorHandle = allocCBVSRVUAVDescriptors();

			GraphicsDevice::get()->CreateConstantBufferView(&desc, descriptorHandle.getCurrentCPUHandle());

			gpuAddress = buffer->getGPUAddress();

			bufferIndex = descriptorHandle.getCurrentIndex();
		}
	}

	ImmutableCBuffer::~ImmutableCBuffer()
	{
		if (buffer)
		{
			delete buffer;
		}
	}

	D3D12Resource::ShaderResourceDesc ImmutableCBuffer::getBufferIndex() const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::BUFFER;
		desc.state = D3D12Resource::ShaderResourceDesc::CBV;
		desc.resourceIndex = bufferIndex;
		desc.bufferDesc.buffer = buffer;

		return desc;
	}

	D3D12_GPU_VIRTUAL_ADDRESS ImmutableCBuffer::getGPUAddress() const
	{
		return gpuAddress;
	}

	D3D12Resource::Buffer* ImmutableCBuffer::getBuffer() const
	{
		return buffer;
	}

	void ImmutableCBuffer::copyDescriptors()
	{
		const D3D12Core::DescriptorHandle shaderVisibleHandle = copyToResourceHeap();

		bufferIndex = shaderVisibleHandle.getCurrentIndex();
	}
}
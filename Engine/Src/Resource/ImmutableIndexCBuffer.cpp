#include<Gear/Resource/ImmutableIndexCBuffer.h>

namespace Gear::Resource
{
	ImmutableIndexCBuffer::~ImmutableIndexCBuffer()
	{
	}

	D3D12Resource::Buffer* ImmutableIndexCBuffer::getBuffer() const
	{
		return buffer.get();
	}

	D3D12_GPU_VIRTUAL_ADDRESS ImmutableIndexCBuffer::getGPUAddress() const
	{
		return *gpuAddress;
	}

	const std::vector<ShaderResourceDesc>& ImmutableIndexCBuffer::getShaderResourceDescs() const
	{
		return shaderResourceDescs;
	}

	uint64_t ImmutableIndexCBuffer::getUpdateSize() const
	{
		return shaderResourceDescs.size() * sizeof(uint32_t);
	}
}

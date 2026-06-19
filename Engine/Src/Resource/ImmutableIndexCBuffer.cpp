#include<Gear/Resource/ImmutableIndexCBuffer.h>

namespace Gear::Resource
{
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
}

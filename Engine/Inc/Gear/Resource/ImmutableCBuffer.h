#pragma once

#ifndef _GEAR_RESOURCE_IMMUTABLECBUFFER_H_
#define _GEAR_RESOURCE_IMMUTABLECBUFFER_H_

#include<Gear/Core/D3D12Resource/Buffer.h>

#include"ResourceBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(ImmutableCBuffer);

	class ImmutableCBuffer :public ResourceBase
	{
	public:

		ImmutableCBuffer() = delete;

		ImmutableCBuffer(D3D12Resource::BufferPtr bufferPtr, const bool persistent);

		ImmutableCBuffer(const ImmutableCBuffer&);

		~ImmutableCBuffer();

		ShaderResourceDesc getBufferIndex() const;

		D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;

		D3D12Resource::Buffer* getBuffer() const;

		bool copyDescriptors() override;

	protected:

		SharedPtr<D3D12_GPU_VIRTUAL_ADDRESS> gpuAddress;

		SharedPtr<uint32_t> bufferIndex;

		D3D12Resource::BufferPtr buffer;

	};
}

#endif // !_GEAR_RESOURCE_IMMUTABLECBUFFER_H_
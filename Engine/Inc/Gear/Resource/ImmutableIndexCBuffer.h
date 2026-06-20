#pragma once

#ifndef _GEAR_RESOURCE_IMMUTABLEINDEXCBUFFER_H_
#define _GEAR_RESOURCE_IMMUTABLEINDEXCBUFFER_H_

#include<Gear/Core/D3D12Resource/Buffer.h>

#include"ResourceBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(ImmutableIndexCBuffer);

	using ResourceDescPair = std::pair<ResourceBase*, ShaderResourceDesc>;

	template<size_t N>
	static constexpr uint64_t getCBufferByteSizeFromPairs(const ResourceDescPair(&pairs)[N])
	{
		constexpr uint64_t byteSize = sizeof(uint32_t) * N;

		//64个资源
		if constexpr (byteSize <= 256ull)
		{
			return 256ull;
		}

		//128个资源
		if constexpr (byteSize <= 512ull)
		{
			return 512ull;
		}

		//256个资源
		if constexpr (byteSize <= 1024ull)
		{
			return 1024ull;
		}

		//512个资源
		if constexpr (byteSize <= 2048ull)
		{
			return 2048ull;
		}

		//1024个资源（应该只需要这么多，真的会有1024个资源以上的需求吗？）
		if constexpr (byteSize <= 4096ull)
		{
			return 4096ull;
		}
	}

	class ImmutableIndexCBuffer
	{
	public:

		template<size_t N>
		ImmutableIndexCBuffer(const ResourceDescPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr);

		virtual ~ImmutableIndexCBuffer();

		D3D12Resource::Buffer* getBuffer() const;

		D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;

		const std::vector<ShaderResourceDesc>& getShaderResourceDescs() const;

		uint64_t getUpdateSize() const;

	protected:

		template<size_t N>
		void updateShaderResourceDescs(const ResourceDescPair(&pairs)[N]);

		D3D12Resource::BufferPtr buffer;

		//用于资源状态转变以及索引提取
		std::vector<ShaderResourceDesc> shaderResourceDescs;

	private:

		//用于绑定
		SharedPtr<D3D12_GPU_VIRTUAL_ADDRESS> gpuAddress;

	};

	template<size_t N>
	inline ImmutableIndexCBuffer::ImmutableIndexCBuffer(const ResourceDescPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr) :
		buffer(std::move(bufferPtr)), gpuAddress(buffer ? makeShared<D3D12_GPU_VIRTUAL_ADDRESS>(buffer->getGPUAddress()) : makeShared<D3D12_GPU_VIRTUAL_ADDRESS>())
	{
		updateShaderResourceDescs(pairs);
	}

	template<size_t N>
	inline void ImmutableIndexCBuffer::updateShaderResourceDescs(const ResourceDescPair(&pairs)[N])
	{
		if (N != shaderResourceDescs.size())
		{
			shaderResourceDescs.resize(N);
		}

		for (uint32_t i = 0; i < N; i++)
		{
			shaderResourceDescs[i] = pairs[i].second;
		}
	}
}

#define RESDESCPAIR(_res_,_srd_) Gear::Resource::ResourceDescPair(_res_.get(), _srd_)

#endif // !_GEAR_RESOURCE_IMMUTABLEINDEXCBUFFER_H_

#pragma once

#ifndef _GEAR_RESOURCE_IMMUTABLEINDEXCBUFFER_H_
#define _GEAR_RESOURCE_IMMUTABLEINDEXCBUFFER_H_

#include<Gear/Core/D3D12Resource/Buffer.h>

#include"ResourceBase.h"

#include<functional>

namespace Gear::Resource
{
	CREATESAFETYPE(ImmutableIndexCBuffer);

	using ResourceIndexPair = std::pair<ResourceBase*, std::function<ShaderResourceDesc(void)>>;

	template<size_t N>
	static constexpr uint64_t getCBufferByteSizeFromPairs(const ResourceIndexPair(&pairs)[N])
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
		ImmutableIndexCBuffer(const ResourceIndexPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr);

		D3D12Resource::Buffer* getBuffer() const;

		D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;

		const std::vector<ShaderResourceDesc>& getShaderResourceDescs() const;

	protected:

		template<size_t N>
		void updateShaderResourceDescs(const ResourceIndexPair(&pairs)[N]);

	private:

		D3D12Resource::BufferPtr buffer;

		//用于绑定
		SharedPtr<D3D12_GPU_VIRTUAL_ADDRESS> gpuAddress;

		//仅用于状态转变
		std::vector<ShaderResourceDesc> shaderResourceDescs;

	};

	template<size_t N>
	inline ImmutableIndexCBuffer::ImmutableIndexCBuffer(const ResourceIndexPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr) :
		buffer(std::move(bufferPtr)), gpuAddress(buffer ? makeShared<D3D12_GPU_VIRTUAL_ADDRESS>(buffer->getGPUAddress()) : makeShared<D3D12_GPU_VIRTUAL_ADDRESS>())
	{
		updateShaderResourceDescs(pairs);
	}

	template<size_t N>
	inline void ImmutableIndexCBuffer::updateShaderResourceDescs(const ResourceIndexPair(&pairs)[N])
	{
		if (N != shaderResourceDescs.size())
		{
			shaderResourceDescs.resize(N);
		}

		for (uint32_t i = 0; i < N; i++)
		{
			shaderResourceDescs[i] = pairs->second();
		}
	}
}

#define CREATEINDEXPAIR(_res_,_func_) Gear::Resource::ResourceIndexPair(_res_.get(),[&]{ return _func_; })

#endif // !_GEAR_RESOURCE_IMMUTABLEINDEXCBUFFER_H_

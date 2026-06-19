#pragma once

#ifndef _GEAR_CORE_INDEXCBUFFERBASE_H_
#define _GEAR_CORE_INDEXCBUFFERBASE_H_

#include<Gear/Resource/ResourceBase.h>

#include<functional>

namespace Gear::Core
{
	using ResourceIndexPair = std::pair<Resource::ResourceBase*, std::function<D3D12Resource::ShaderResourceDesc(void)>>;

	CREATESAFETYPE(IndexCBufferBase);

	class IndexCBufferBase
	{
	public:

		template<size_t N>
		IndexCBufferBase(const ResourceIndexPair(&pairs)[N]);

		template<size_t N>
		void updateIndexPairs(const ResourceIndexPair(&pairs)[N]);

		void updateResourceIndices();

		template<size_t N>
		static constexpr uint64_t getCBufferByteSize(const ResourceIndexPair(&pairs)[N]);

	protected:

		std::vector<uint32_t> resourceIndices;

	private:

		bool hasNonPersistentResource;

		std::vector<D3D12Resource::ShaderResourceDesc> shaderResourceDescs;

		std::vector<ResourceIndexPair> indexPairs;

	};

	template<size_t N>
	inline IndexCBufferBase::IndexCBufferBase(const ResourceIndexPair(&pairs)[N])
	{
		updateIndexPairs(pairs);
	}

	template<size_t N>
	inline void IndexCBufferBase::updateIndexPairs(const ResourceIndexPair(&pairs)[N])
	{
		const uint64_t byteSize = getCBufferByteSize(pairs);

		const uint32_t numElement = byteSize / sizeof(uint32_t);

		if (numElement != resourceIndices.size())
		{
			resourceIndices = std::vector<uint32_t>(numElement);
		}

		hasNonPersistentResource = false;

		for (uint32_t i = 0; i < N; i++)
		{
			if (!(pairs[i].first->getPersistent()))
			{
				hasNonPersistentResource = true;

				break;
			}
		}

		if (N != shaderResourceDescs.size())
		{
			shaderResourceDescs = std::vector<D3D12Resource::ShaderResourceDesc>(N);
		}

		indexPairs = std::vector<ResourceIndexPair>(pairs, pairs + N);

		if (!hasNonPersistentResource)
		{
			for (uint32_t i = 0; i < indexPairs.size(); i++)
			{
				shaderResourceDescs[i] = indexPairs[i].second();

				resourceIndices[i] = shaderResourceDescs[i].resourceIndex;
			}
		}
	}

	template<size_t N>
	inline constexpr uint64_t IndexCBufferBase::getCBufferByteSize(const ResourceIndexPair(&pairs)[N])
	{
		constexpr uint64_t byteSize = sizeof(uint32_t) * N;

		if constexpr (byteSize <= 256ull)
		{
			return 256ull;
		}

		if constexpr (byteSize <= 512ull)
		{
			return 512ull;
		}

		if constexpr (byteSize <= 1024ull)
		{
			return 1024ull;
		}

		if constexpr (byteSize <= 2048ull)
		{
			return 2048ull;
		}
	}
}

#define CREATEINDEXPAIR(_res_,_func_) Gear::Core::ResourceIndexPair(_res_.get(),[&]{ return _func_; })

#endif // !_GEAR_CORE_INDEXCBUFFERBASE_H_

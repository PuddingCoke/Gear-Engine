#pragma once

#ifndef _GEAR_RESOURCE_DEFAULTINDEXCBUFFERBASE_H_
#define _GEAR_RESOURCE_DEFAULTINDEXCBUFFERBASE_H_

#include"ImmutableIndexCBuffer.h"

namespace Gear::Resource
{
	class DefaultIndexCBufferBase :public ImmutableIndexCBuffer
	{
	public:

		template<size_t N>
		DefaultIndexCBufferBase(const ResourceDescPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr);

		template<size_t N>
		void update(const ResourceDescPair(&pairs)[N]);

		bool updateResourceIndices();

	protected:

		std::vector<uint32_t> resourceIndices;

		bool resourceIndicesUpdated;

	private:

		//用于获取最新的资源索引
		std::vector<std::pair<ResourceBase*, uint32_t>> nonPersistentResourcePairs;

	};

	template<size_t N>
	inline DefaultIndexCBufferBase::DefaultIndexCBufferBase(const ResourceDescPair(&pairs)[N], Core::D3D12Resource::BufferPtr bufferPtr) :
		ImmutableIndexCBuffer(pairs, std::move(bufferPtr)), resourceIndicesUpdated(false)
	{
		const uint64_t byteSize = getCBufferByteSizeFromPairs(pairs);

		const uint32_t numElement = byteSize / sizeof(uint32_t);

		if (numElement != resourceIndices.size())
		{
			resourceIndices = std::vector<uint32_t>(numElement, UINT32_MAX);
		}

		nonPersistentResourcePairs.clear();

		for (uint32_t i = 0; i < N; i++)
		{
			if (pairs[i].first->getPersistent())
			{
				resourceIndices[i] = *pairs[i].second.resourceIndex;
			}
			else
			{
				nonPersistentResourcePairs.push_back(std::pair<ResourceBase*, uint32_t>(pairs[i].first, i));
			}
		}
	}

	template<size_t N>
	inline void DefaultIndexCBufferBase::update(const ResourceDescPair(&pairs)[N])
	{
		resourceIndicesUpdated = true;

		const uint64_t byteSize = getCBufferByteSizeFromPairs(pairs);

		const uint32_t numElement = byteSize / sizeof(uint32_t);

		if (numElement != resourceIndices.size())
		{
			resourceIndices = std::vector<uint32_t>(numElement, UINT32_MAX);
		}

		nonPersistentResourcePairs.clear();

		for (uint32_t i = 0; i < N; i++)
		{
			if (pairs[i].first->getPersistent())
			{
				resourceIndices[i] = *pairs[i].second.resourceIndex;
			}
			else
			{
				nonPersistentResourcePairs.push_back(std::pair<ResourceBase*, uint32_t>(pairs[i].first, i));
			}
		}

		updateShaderResourceDescs(pairs);
	}
}

#endif // !_GEAR_RESOURCE_DEFAULTINDEXCBUFFERBASE_H_

#pragma once

#ifndef _GEAR_RESOURCE_INDEXCBUFFERBASE_H_
#define _GEAR_RESOURCE_INDEXCBUFFERBASE_H_

#include"ImmutableIndexCBuffer.h"

namespace Gear::Resource
{
	class IndexCBufferBase
	{
	public:

		template<size_t N>
		IndexCBufferBase(const ResourceIndexPair(&pairs)[N]);

		template<size_t N>
		void updateIndexPairs(const ResourceIndexPair(&pairs)[N]);

		void updateResourceIndices();

	protected:

		std::vector<uint32_t> resourceIndices;

	private:

		//仅用于取出索引
		std::vector<ResourceIndexPair> indexPairs;

		std::vector<uint32_t> nonPersistentIndices;

	};

	template<size_t N>
	inline IndexCBufferBase::IndexCBufferBase(const ResourceIndexPair(&pairs)[N])
	{
		updateIndexPairs(pairs);
	}

	template<size_t N>
	inline void IndexCBufferBase::updateIndexPairs(const ResourceIndexPair(&pairs)[N])
	{
		const uint64_t byteSize = getCBufferByteSizeFromPairs(pairs);

		const uint32_t numElement = byteSize / sizeof(uint32_t);

		if (numElement != resourceIndices.size())
		{
			resourceIndices = std::vector<uint32_t>(numElement);
		}

		indexPairs = std::vector<ResourceIndexPair>(pairs, pairs + N);

		nonPersistentIndices.clear();

		for (uint32_t i = 0; i < indexPairs.size(); i++)
		{
			if (indexPairs[i].first->getPersistent())
			{
				resourceIndices[i] = indexPairs[i].second().resourceIndex;
			}
			else
			{
				nonPersistentIndices.push_back(i);
			}
		}
	}
}

#endif // !_GEAR_RESOURCE_INDEXCBUFFERBASE_H_

#include<Gear/Resource/IndexCBufferBase.h>

namespace Gear::Resource
{
	void IndexCBufferBase::updateResourceIndices()
	{
		//只更新非持久性资源的索引
		for (const uint32_t i : nonPersistentIndices)
		{
			indexPairs[i].first->copyDescriptors();

			resourceIndices[i] = indexPairs[i].second().resourceIndex;
		}
	}
}
#include<Gear/Core/IndexCBufferBase.h>

namespace Gear::Core
{
	void IndexCBufferBase::updateResourceIndices()
	{
		if (hasNonPersistentResource)
		{
			for (uint32_t i = 0; i < indexPairs.size(); i++)
			{
				if (!(indexPairs[i].first->getPersistent()))
				{
					indexPairs[i].first->copyDescriptors();
				}

				shaderResourceDescs[i] = indexPairs[i].second();

				resourceIndices[i] = shaderResourceDescs[i].resourceIndex;
			}
		}
	}
}
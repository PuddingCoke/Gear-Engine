#include<Gear/Resource/DefaultIndexCBufferBase.h>

namespace Gear::Resource
{
	bool DefaultIndexCBufferBase::updateResourceIndices()
	{
		bool anyCopied = false;

		if (nonPersistentResourcePairs.size())
		{
			for (std::pair<ResourceBase*, uint32_t> pair : nonPersistentResourcePairs)
			{
				pair.first->copyDescriptors();

				if (resourceIndices[pair.second] != *shaderResourceDescs[pair.second].resourceIndex)
				{
					resourceIndices[pair.second] = *shaderResourceDescs[pair.second].resourceIndex;

					anyCopied = true;
				}
			}
		}

		return anyCopied;
	}
}
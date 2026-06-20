#include<Gear/Resource/DefaultIndexCBuffer.h>

namespace Gear::Resource
{
	DefaultIndexCBuffer::~DefaultIndexCBuffer()
	{
		if (uploadHeaps.get())
		{
			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				if (uploadHeaps[i].get())
				{
					uploadHeaps[i]->unmap();
				}
			}
		}
	}

	bool DefaultIndexCBuffer::getNeedUpdate()
	{
		const bool needUpdate = updateResourceIndices() || resourceIndicesUpdated;

		resourceIndicesUpdated = false;

		return needUpdate;
	}

	DefaultIndexCBuffer::UpdateStruct DefaultIndexCBuffer::getUpdateStruct()
	{
		memcpy(dataPtrs[Graphics::getFrameIndex()], resourceIndices.data(), getUpdateSize());

		const UpdateStruct updateStruct = { buffer.get(),uploadHeaps[Graphics::getFrameIndex()].get() };

		return updateStruct;
	}
}
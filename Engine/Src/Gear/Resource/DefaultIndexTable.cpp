#include <Gear/Resource/DefaultIndexTable.h>

namespace Gear::Resource
{
	DefaultIndexTable::~DefaultIndexTable()
	{
	}

	Core::D3D12Resource::Buffer* DefaultIndexTable::getBuffer() const
	{
		return cbuffer->getBuffer();
	}

	D3D12_GPU_VIRTUAL_ADDRESS DefaultIndexTable::getGPUAddress() const
	{
		return cbuffer->getGPUAddress();
	}

	IndexTable::UpdateStruct DefaultIndexTable::getUpdateStruct() const
	{
		const DefaultCBuffer::UpdateStruct updateStruct =
			cbuffer->getUpdateStruct(shadowIndices, byteSize);

		return { updateStruct.buffer, updateStruct.uploadHeap, byteSize };
	}

	bool DefaultIndexTable::updateIndices()
	{
		if (!getUserUpdate())
		{
			return false;
		}

		setUserUpdate(false);

		bool indicesChanged = false;

		const uint32_t numShaderResourceDescs = static_cast<uint32_t>(getNumShaderResourceDescs());

		const ShaderResourceDesc* const shaderResourceDescsParam = getShaderResourceDescsData();

		for (uint32_t i = 0; i < numShaderResourceDescs; i++)
		{
			if (shadowIndices[i] != shaderResourceDescsParam[i].resourceIndex)
			{
				shadowIndices[i] = shaderResourceDescsParam[i].resourceIndex;

				indicesChanged = true;
			}
		}

		return indicesChanged;
	}
}

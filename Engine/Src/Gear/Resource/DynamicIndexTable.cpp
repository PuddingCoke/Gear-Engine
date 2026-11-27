#include <Gear/Resource/DynamicIndexTable.h>

namespace Gear::Resource
{
	DynamicIndexTable::~DynamicIndexTable()
	{
	}

	Core::D3D12Resource::Buffer* DynamicIndexTable::getBuffer() const
	{
		//动态类型的索引表没有独立的常量缓冲
		return nullptr;
	}

	D3D12_GPU_VIRTUAL_ADDRESS DynamicIndexTable::getGPUAddress() const
	{
		return cbuffer->getGPUAddress();
	}

	IndexTable::UpdateStruct DynamicIndexTable::getUpdateStruct() const
	{
		THROWLOG(LOGERROR() << "动态类型的索引表不需要更新缓冲区！");
	}

	bool DynamicIndexTable::updateIndices()
	{
		if (!getUserUpdate())
		{
			return false;
		}

		setUserUpdate(false);

		uint32_t* const shadowIndices = static_cast<uint32_t*>(cbuffer->getDataPtr());

		const uint32_t numShaderResourceDescs = static_cast<uint32_t>(getNumShaderResourceDescs());

		const ShaderResourceDesc* const shaderResourceDescsParam = getShaderResourceDescsData();

		for (uint32_t i = 0; i < numShaderResourceDescs; i++)
		{
			shadowIndices[i] = shaderResourceDescsParam[i].resourceIndex;
		}

		//影子缓冲就是权威数据，DynamicCBufferManager每帧会把已占用的子区域拷贝到GPU
		return false;
	}
}

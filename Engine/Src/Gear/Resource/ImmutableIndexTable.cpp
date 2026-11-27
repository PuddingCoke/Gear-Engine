#include <Gear/Resource/ImmutableIndexTable.h>

namespace Gear::Resource
{
	ImmutableIndexTable::~ImmutableIndexTable()
	{
	}

	Core::D3D12Resource::Buffer* ImmutableIndexTable::getBuffer() const
	{
		return cbuffer->getBuffer();
	}

	D3D12_GPU_VIRTUAL_ADDRESS ImmutableIndexTable::getGPUAddress() const
	{
		return cbuffer->getGPUAddress();
	}

	IndexTable::UpdateStruct ImmutableIndexTable::getUpdateStruct() const
	{
		THROWLOG(LOGERROR() << "不可变类型的索引表永远不会更新缓冲区！");
	}

	bool ImmutableIndexTable::updateIndices()
	{
		//不可变类型的索引表不会被放进更新队列，这里只是防御性处理
		setUserUpdate(false);

		return false;
	}
}

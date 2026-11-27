#include <Gear/Resource/IndexTable.h>

namespace Gear::Resource
{
	IndexTable::IndexTable() :
		userUpdate(false),
		inUpdateList(false),
		inCheckedList(false)
	{
	}

	IndexTable::~IndexTable()
	{
	}

	bool IndexTable::getUserUpdate() const
	{
		return userUpdate;
	}

	void IndexTable::setUserUpdate(const bool userUpdateParam)
	{
		userUpdate = userUpdateParam;
	}

	const ShaderResourceDesc* IndexTable::getShaderResourceDescsData() const
	{
		return shaderResourceDescs.data();
	}

	uint64_t IndexTable::getNumShaderResourceDescs() const
	{
		return shaderResourceDescs.size();
	}

	bool IndexTable::getInUpdateList() const
	{
		return inUpdateList;
	}

	bool IndexTable::getInCheckedList() const
	{
		return inCheckedList;
	}

	void IndexTable::pushToUpdateList()
	{
		inUpdateList = true;
	}

	void IndexTable::popFromUpdateList()
	{
		inUpdateList = false;
	}

	void IndexTable::pushToCheckedList()
	{
		inCheckedList = true;
	}

	void IndexTable::popFromCheckedList()
	{
		inCheckedList = false;
	}
}

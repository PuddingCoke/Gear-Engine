#include <Gear/Core/GraphicsCommand/ClearUnorderedAccessCommand.h>

namespace Gear::Core
{
	ClearUnorderedAccessCommand::ClearUnorderedAccessCommand(const Resource::UAVClearDesc& desc,
	                                                         const float values[4])
	{
		set(desc, values);
	}

	ClearUnorderedAccessCommand::ClearUnorderedAccessCommand(const Resource::UAVClearDesc& desc,
	                                                         const uint32_t values[4])
	{
		set(desc, values);
	}

	void ClearUnorderedAccessCommand::set(const Resource::UAVClearDesc& desc, const float values[4])
	{
		this->desc = desc;

		memcpy(floatValues, values, sizeof(float) * _countof(floatValues));

		clearFloat = true;
	}

	void ClearUnorderedAccessCommand::set(const Resource::UAVClearDesc& desc, const uint32_t values[4])
	{
		this->desc = desc;

		memcpy(uintValues, values, sizeof(uint32_t) * _countof(uintValues));

		clearFloat = false;
	}

	void ClearUnorderedAccessCommand::execute(GraphicsCommandExecuter& executer)
	{
		if (clearFloat)
		{
			executer.clearUnorderedAccess(desc, floatValues);
		}
		else
		{
			executer.clearUnorderedAccess(desc, uintValues);
		}
	}
}

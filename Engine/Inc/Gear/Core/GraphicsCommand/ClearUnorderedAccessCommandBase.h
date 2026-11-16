#ifndef _GEAR_CORE_CLEARUNORDEREDACCESSCOMMANDBASE_H_
#define _GEAR_CORE_CLEARUNORDEREDACCESSCOMMANDBASE_H_

#include <Gear/Resource/PipelineResourceDesc.h>

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<bool ClearFloat>
	class ClearUnorderedAccessCommandBase :
		public GraphicsCommand,
		public CommandPool<ClearUnorderedAccessCommandBase<ClearFloat>>
	{
	public:

		void set(const Resource::UAVClearDesc& descParam, const void* const dataParam)
		{
			desc = descParam;

			if constexpr (ClearFloat)
			{
				memcpy(floatValues, dataParam, sizeof(float) * _countof(floatValues));
			}
			else
			{
				memcpy(uintValues, dataParam, sizeof(uint32_t) * _countof(uintValues));
			}
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			if constexpr (ClearFloat)
			{
				executer.clearUnorderedAccess(desc, floatValues);
			}
			else
			{
				executer.clearUnorderedAccess(desc, uintValues);
			}
		}

	private:

		Resource::UAVClearDesc desc;

		union
		{
			float floatValues[4];

			uint32_t uintValues[4];
		};
	};

	using ClearUnorderedAccessUintCommand = ClearUnorderedAccessCommandBase<false>;

	using ClearUnorderedAccessFloatCommand = ClearUnorderedAccessCommandBase<true>;

}

#endif // !_GEAR_CORE_CLEARUNORDEREDACCESSCOMMANDBASE_H_

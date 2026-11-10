#ifndef _GEAR_CORE_CLEARUNORDEREDACCESSCOMMAND_H_
#define _GEAR_CORE_CLEARUNORDEREDACCESSCOMMAND_H_

#include <Gear/Resource/PipelineResourceDesc.h>

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class ClearUnorderedAccessCommand : public GraphicsCommand
	{
	public:

		ClearUnorderedAccessCommand() = delete;

		ClearUnorderedAccessCommand(const Resource::UAVClearDesc& desc, const float values[4]);

		ClearUnorderedAccessCommand(const Resource::UAVClearDesc& desc, const uint32_t values[4]);

		void set(const Resource::UAVClearDesc& desc, const float values[4]);

		void set(const Resource::UAVClearDesc& desc, const uint32_t values[4]);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		Resource::UAVClearDesc desc;

		union
		{
			float floatValues[4];

			uint32_t uintValues[4];
		};

		bool clearFloat;
	};
}

#endif // !_GEAR_CORE_CLEARUNORDEREDACCESSCOMMAND_H_

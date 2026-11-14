#ifndef _GEAR_CORE_SETPRIMITIVETOPOLOGYCOMMAND_H_
#define _GEAR_CORE_SETPRIMITIVETOPOLOGYCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetPrimitiveTopologyCommand : public GraphicsCommand, public CommandPool<SetPrimitiveTopologyCommand>
	{
	public:

		void set(const D3D12_PRIMITIVE_TOPOLOGY topology);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		D3D12_PRIMITIVE_TOPOLOGY topology;
	};
}

#endif

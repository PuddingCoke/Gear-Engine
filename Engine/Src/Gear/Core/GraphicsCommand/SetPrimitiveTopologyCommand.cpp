#include <Gear/Core/GraphicsCommand/SetPrimitiveTopologyCommand.h>

namespace Gear::Core
{

	void SetPrimitiveTopologyCommand::set(const D3D12_PRIMITIVE_TOPOLOGY topologyParam)
	{
		topology = topologyParam;
	}

	void SetPrimitiveTopologyCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setPrimitiveTopology(topology);
	}

}

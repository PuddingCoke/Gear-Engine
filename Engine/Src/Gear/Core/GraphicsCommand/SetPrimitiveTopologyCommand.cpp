#include<Gear/Core/GraphicsCommand/SetPrimitiveTopologyCommand.h>

namespace Gear::Core
{
	SetPrimitiveTopologyCommand::SetPrimitiveTopologyCommand(const D3D12_PRIMITIVE_TOPOLOGY topology) :
		topology(topology)
	{
	}

	void SetPrimitiveTopologyCommand::set(const D3D12_PRIMITIVE_TOPOLOGY topology)
	{
		this->topology = topology;
	}

	void SetPrimitiveTopologyCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setPrimitiveTopology(topology);
	}
}

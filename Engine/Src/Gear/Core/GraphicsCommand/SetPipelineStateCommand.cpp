#include <Gear/Core/GraphicsCommand/SetPipelineStateCommand.h>

namespace Gear::Core
{
	SetPipelineStateCommand::SetPipelineStateCommand(D3D12Core::GraphicsState& graphicsPipelineState)
	{
		set(graphicsPipelineState);
	}

	SetPipelineStateCommand::SetPipelineStateCommand(D3D12Core::ComputeState& computePipelineState)
	{
		set(computePipelineState);
	}

	void SetPipelineStateCommand::set(D3D12Core::GraphicsState& graphicsPipelineState)
	{
		graphicsState = &graphicsPipelineState;

		isGraphicsPipelineState = true;
	}

	void SetPipelineStateCommand::set(D3D12Core::ComputeState& computePipelineState)
	{
		computeState = &computePipelineState;

		isGraphicsPipelineState = false;
	}

	void SetPipelineStateCommand::execute(GraphicsCommandExecuter& executer)
	{
		if (isGraphicsPipelineState)
		{
			executer.setPipelineState(*graphicsState);
		}
		else
		{
			executer.setPipelineState(*computeState);
		}
	}
}

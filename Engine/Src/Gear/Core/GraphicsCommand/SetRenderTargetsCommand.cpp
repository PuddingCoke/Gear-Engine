#include <Gear/Core/GraphicsCommand/SetRenderTargetsCommand.h>

namespace Gear::Core
{
	SetRenderTargetsCommand::SetRenderTargetsCommand(const uint64_t renderTargetDescsPtrOffset,
	                                                 const uint32_t numRenderTargetsDescs,
	                                                 const Resource::DepthStencilDesc& depthStencilDesc)
	{
		set(renderTargetDescsPtrOffset, numRenderTargetsDescs, depthStencilDesc);
	}

	void SetRenderTargetsCommand::set(const uint64_t renderTargetDescsPtrOffset,
	                                  const uint32_t numRenderTargetsDescs,
	                                  const Resource::DepthStencilDesc& depthStencilDesc)
	{
		this->renderTargetDescsPtrOffset = renderTargetDescsPtrOffset;

		this->numRenderTargetsDescs = numRenderTargetsDescs;

		this->depthStencilDesc = depthStencilDesc;
	}

	void SetRenderTargetsCommand::execute(GraphicsCommandExecuter& executer)
	{
		if (renderTargetDescsPtrOffset == UINT64_MAX)
		{
			executer.setDefRenderTarget();
		}
		else
		{
			executer.setRenderTargets(renderTargetDescsPtrOffset, numRenderTargetsDescs, depthStencilDesc);
		}
	}
}

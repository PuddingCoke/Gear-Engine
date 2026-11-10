#include <Gear/Core/GraphicsCommand/ClearDepthStencilCommand.h>

namespace Gear::Core
{
	ClearDepthStencilCommand::ClearDepthStencilCommand(const Resource::DepthStencilDesc& desc,
	                                                   const D3D12_CLEAR_FLAGS flags,
	                                                   const float depth,
	                                                   const uint8_t stencil,
	                                                   const bool instant)
	{
		set(desc, flags, depth, stencil, instant);
	}

	void ClearDepthStencilCommand::set(const Resource::DepthStencilDesc& desc,
	                                   const D3D12_CLEAR_FLAGS flags,
	                                   const float depth,
	                                   const uint8_t stencil,
	                                   const bool instant)
	{
		this->desc = desc;

		this->flags = flags;

		this->depth = depth;

		this->stencil = stencil;

		this->instant = instant;
	}

	void ClearDepthStencilCommand::execute(GraphicsCommandExecuter& executer)
	{
		if (instant)
		{
			executer.clearDepthStencilInstant(desc, flags, depth, stencil);
		}
		else
		{
			executer.clearDepthStencil(flags, depth, stencil);
		}
	}
}

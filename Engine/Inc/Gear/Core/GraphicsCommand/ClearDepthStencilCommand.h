#ifndef _GEAR_CORE_CLEARDEPTHSTENCILCOMMAND_H_
#define _GEAR_CORE_CLEARDEPTHSTENCILCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class ClearDepthStencilCommand : public GraphicsCommand
	{
	public:

		ClearDepthStencilCommand() = delete;

		ClearDepthStencilCommand(const Resource::DepthStencilDesc& desc,
		                         const D3D12_CLEAR_FLAGS flags,
		                         const float depth,
		                         const uint8_t stencil,
		                         const bool instant);

		void set(const Resource::DepthStencilDesc& desc,
		         const D3D12_CLEAR_FLAGS flags,
		         const float depth,
		         const uint8_t stencil,
		         const bool instant);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		Resource::DepthStencilDesc desc;

		D3D12_CLEAR_FLAGS flags;

		float depth;

		uint8_t stencil;

		bool instant;
	};
}

#endif

#ifndef _GEAR_CORE_CLEARDEPTHSTENCILCOMMANDBASE_H_
#define _GEAR_CORE_CLEARDEPTHSTENCILCOMMANDBASE_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<bool Instant>
	class ClearDepthStencilCommandBase :
		public GraphicsCommand,
		public CommandPool<ClearDepthStencilCommandBase<Instant>>
	{
	public:

		void set(const Resource::DepthStencilDesc& descParam,
		         const D3D12_CLEAR_FLAGS flagsParam,
		         const float depthParam,
		         const uint8_t stencilParam)
		{
			desc = descParam;

			flags = flagsParam;

			depth = depthParam;

			stencil = stencilParam;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			if constexpr (Instant)
			{
				executer.clearDepthStencilInstant(desc, flags, depth, stencil);
			}
			else
			{
				executer.clearDepthStencil(flags, depth, stencil);
			}
		}

	private:

		Resource::DepthStencilDesc desc;

		D3D12_CLEAR_FLAGS flags;

		float depth;

		uint8_t stencil;
	};

	using ClearDepthStencilCommand = ClearDepthStencilCommandBase<false>;

	using ClearDepthStencilInstantCommand = ClearDepthStencilCommandBase<true>;

}

#endif

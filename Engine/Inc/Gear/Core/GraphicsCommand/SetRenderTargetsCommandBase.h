#ifndef _GEAR_CORE_SETRENDERTARGETSCOMMANDBASE_H_
#define _GEAR_CORE_SETRENDERTARGETSCOMMANDBASE_H_

#include <Gear/Resource/PipelineResourceDesc.h>

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<bool SetDefRenderTarget>
	class SetRenderTargetsCommandBase :
		public GraphicsCommand,
		public CommandPool<SetRenderTargetsCommandBase<SetDefRenderTarget>>
	{
	public:

		void set(const uint64_t renderTargetDescsPtrOffsetParam,
		         const uint32_t numRenderTargetsDescsParam,
		         const Resource::DepthStencilDesc& depthStencilDescParam)
		{
			renderTargetDescsPtrOffset = renderTargetDescsPtrOffsetParam;

			numRenderTargetsDescs = numRenderTargetsDescsParam;

			depthStencilDesc = depthStencilDescParam;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			if constexpr (SetDefRenderTarget)
			{
				executer.setDefRenderTarget();
			}
			else
			{
				executer.setRenderTargets(renderTargetDescsPtrOffset, numRenderTargetsDescs, depthStencilDesc);
			}
		}

	private:

		uint64_t renderTargetDescsPtrOffset;

		uint32_t numRenderTargetsDescs;

		Resource::DepthStencilDesc depthStencilDesc;
	};

	using SetRenderTargetsCommand = SetRenderTargetsCommandBase<false>;

	using SetDefRenderTargetCommand = SetRenderTargetsCommandBase<true>;

}

#endif // _GEAR_CORE_SETRENDERTARGETSCOMMANDBASE_H_

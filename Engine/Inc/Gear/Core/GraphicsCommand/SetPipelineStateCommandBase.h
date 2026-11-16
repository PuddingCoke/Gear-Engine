#ifndef _GEAR_CORE_SETPPIPELINESTATECOMMANDBASE_H_
#define _GEAR_CORE_SETPPIPELINESTATECOMMANDBASE_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<bool IsGraphicsPipelineState>
	class SetPipelineStateCommandBase :
		public GraphicsCommand,
		public CommandPool<SetPipelineStateCommandBase<IsGraphicsPipelineState>>
	{
	public:

		template<bool D = IsGraphicsPipelineState>
			requires(D)
		void set(D3D12Core::GraphicsState& graphicsPipelineStateParam)
		{
			graphicsState = &graphicsPipelineStateParam;
		}

		template<bool D = IsGraphicsPipelineState>
			requires(!D)
		void set(D3D12Core::ComputeState& computePipelineStateParam)
		{
			computeState = &computePipelineStateParam;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			if constexpr (IsGraphicsPipelineState)
			{
				executer.setPipelineState(*graphicsState);
			}
			else
			{
				executer.setPipelineState(*computeState);
			}
		}

	private:

		union
		{
			D3D12Core::GraphicsState* graphicsState;

			D3D12Core::ComputeState* computeState;
		};
	};

	using SetComputePipelineStateCommand = SetPipelineStateCommandBase<false>;

	using SetGraphicsPipelineStateCommand = SetPipelineStateCommandBase<true>;
}

#endif // !_GEAR_CORE_SETPPIPELINESTATECOMMANDBASE_H_

#ifndef _GEAR_CORE_SETPPIPELINESTATECOMMAND_H_
#define _GEAR_CORE_SETPPIPELINESTATECOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetPipelineStateCommand : public GraphicsCommand
	{
	public:

		SetPipelineStateCommand() = delete;

		SetPipelineStateCommand(D3D12Core::GraphicsState& graphicsPipelineState);

        SetPipelineStateCommand(D3D12Core::ComputeState& computePipelineState);

        void set(D3D12Core::GraphicsState& graphicsPipelineState);

        void set(D3D12Core::ComputeState& computePipelineState);

		void execute(GraphicsCommandExecuter& executer) override;

    private:

        union
		{
			D3D12Core::GraphicsState* graphicsState;

			D3D12Core::ComputeState* computeState;
		};
        
        bool isGraphicsPipelineState;

	};
}

#endif // !_GEAR_CORE_SETPPIPELINESTATECOMMAND_H_

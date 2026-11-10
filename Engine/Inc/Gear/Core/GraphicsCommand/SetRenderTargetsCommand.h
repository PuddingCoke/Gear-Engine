#ifndef _GEAR_CORE_SETRENDERTARGETSCOMMAND_H_
#define _GEAR_CORE_SETRENDERTARGETSCOMMAND_H_

#include <Gear/Resource/PipelineResourceDesc.h>

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetRenderTargetsCommand : public GraphicsCommand
	{
	public:

		SetRenderTargetsCommand() = delete;

		SetRenderTargetsCommand(const uint64_t renderTargetDescsPtrOffset,
		                        const uint32_t numRenderTargetsDescs,
		                        const Resource::DepthStencilDesc& depthStencilDesc);

		void set(const uint64_t renderTargetDescsPtrOffset,
		         const uint32_t numRenderTargetsDescs,
		         const Resource::DepthStencilDesc& depthStencilDesc);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		uint64_t renderTargetDescsPtrOffset;

		uint32_t numRenderTargetsDescs;

		Resource::DepthStencilDesc depthStencilDesc;
	};
}

#endif // _GEAR_CORE_SETRENDERTARGETSCOMMAND_H_

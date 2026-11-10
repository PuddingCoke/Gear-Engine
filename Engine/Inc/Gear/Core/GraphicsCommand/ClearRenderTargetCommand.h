#ifndef _GEAR_CORE_CLEARDEFRENDERTARGETCOMMAND_H_
#define _GEAR_CORE_CLEARDEFRENDERTARGETCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class ClearRenderTargetCommand : public GraphicsCommand
	{
	public:

		ClearRenderTargetCommand() = delete;

		ClearRenderTargetCommand(const Resource::RenderTargetDesc& desc,
		                         const float clearValue[4],
		                         const bool instant);

		void set(const Resource::RenderTargetDesc& desc, const float clearValue[4], const bool instant);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		Resource::RenderTargetDesc desc;

		float clearValue[4];

		bool instant;
	};
}

#endif

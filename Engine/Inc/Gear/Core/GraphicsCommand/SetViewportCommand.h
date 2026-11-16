#ifndef _GEAR_CORE_SETVIEWPORTCOMMAND_H_
#define _GEAR_CORE_SETVIEWPORTCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetViewportCommand : public GraphicsCommand, public CommandPool<SetViewportCommand>
	{
	public:

		void set(const float widthParam, const float heightParam);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		float width;

		float height;
	};
}

#endif

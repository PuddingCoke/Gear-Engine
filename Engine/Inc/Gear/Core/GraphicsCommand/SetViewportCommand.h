#ifndef _GEAR_CORE_SETVIEWPORT_H_
#define _GEAR_CORE_SETVIEWPORT_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetViewportCommand : public GraphicsCommand
	{
	public:

		SetViewportCommand() = delete;

		SetViewportCommand(const float width, const float height);

		void set(const float width, const float height);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		float width;

        float height;
	};
}

#endif
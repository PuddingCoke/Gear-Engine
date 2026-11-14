#include <Gear/Core/GraphicsCommand/SetViewportCommand.h>

namespace Gear::Core
{

	void SetViewportCommand::set(const float width, const float height)
	{
		this->width = width;

		this->height = height;
	}

	void SetViewportCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setViewport(width, height);
	}

}

#include <Gear/Core/GraphicsCommand/SetViewportCommand.h>

namespace Gear::Core
{

	void SetViewportCommand::set(const float widthParam, const float heightParam)
	{
		width = widthParam;

		height = heightParam;
	}

	void SetViewportCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setViewport(width, height);
	}

}

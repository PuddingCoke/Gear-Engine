#include <Gear/Core/GraphicsCommand/SetScissorRectCommand.h>

namespace Gear::Core
{

	void SetScissorRectCommand::set(const int32_t leftParam,
	                                const int32_t topParam,
	                                const int32_t rightParam,
	                                const int32_t bottomParam)
	{
		left = leftParam;

		top = topParam;

		right = rightParam;

		bottom = bottomParam;
	}

	void SetScissorRectCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setScissorRect(left, top, right, bottom);
	}

}

#include <Gear/Core/GraphicsCommand/SetScissorRectCommand.h>

namespace Gear::Core
{
	SetScissorRectCommand::SetScissorRectCommand(const int32_t left,
	                                             const int32_t top,
	                                             const int32_t right,
	                                             const int32_t bottom)
	{
		set(left, top, right, bottom);
	}

	void SetScissorRectCommand::set(const int32_t left,
	                                const int32_t top,
	                                const int32_t right,
	                                const int32_t bottom)
	{
		this->left = left;

		this->top = top;

		this->right = right;

		this->bottom = bottom;
	}

	void SetScissorRectCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setScissorRect(left, top, right, bottom);
	}
}

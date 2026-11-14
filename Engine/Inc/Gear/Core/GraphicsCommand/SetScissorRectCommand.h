#ifndef _GEAR_CORE_SETSCISSORRECTCOMMAND_H_
#define _GEAR_CORE_SETSCISSORRECTCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetScissorRectCommand : public GraphicsCommand, public CommandPool<SetScissorRectCommand>
	{
	public:

		void set(const int32_t left, const int32_t top, const int32_t right, const int32_t bottom);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		int32_t left;

		int32_t top;

		int32_t right;

		int32_t bottom;
	};
}

#endif

#ifndef _GEAR_CORE_SETINDEXBUFFERCOMMAND_H_
#define _GEAR_CORE_SETINDEXBUFFERCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetIndexBufferCommand : public GraphicsCommand, public CommandPool<SetIndexBufferCommand>
	{
	public:

		void set(const Resource::IndexBufferDesc& indexBufferDescParam);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		Resource::IndexBufferDesc indexBufferDesc;
	};
}

#endif

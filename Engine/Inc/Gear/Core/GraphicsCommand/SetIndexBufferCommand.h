#ifndef _GEAR_CORE_SETINDEXBUFFERCOMMAND_H_
#define _GEAR_CORE_SETINDEXBUFFERCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetIndexBufferCommand : public GraphicsCommand
	{
	public:

		SetIndexBufferCommand() = delete;

		SetIndexBufferCommand(const Resource::IndexBufferDesc& indexBufferDesc);

		void set(const Resource::IndexBufferDesc& indexBufferDesc);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		Resource::IndexBufferDesc indexBufferDesc;
	};
}

#endif
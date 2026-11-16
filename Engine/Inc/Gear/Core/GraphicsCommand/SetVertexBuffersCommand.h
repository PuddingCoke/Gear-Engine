#ifndef _GEAR_CORE_SETVERTEXBUFFERSCOMMAND_H_
#define _GEAR_CORE_SETVERTEXBUFFERSCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetVertexBuffersCommand : public GraphicsCommand, public CommandPool<SetVertexBuffersCommand>
	{
	public:

		void set(const uint64_t vertexBufferDescsPtrOffsetParam,
		         const uint32_t numVertexBuffersDescsParam,
		         const uint32_t startSlotParam);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		uint64_t vertexBufferDescsPtrOffset;

		uint32_t numVertexBufferDescs;

		uint32_t startSlot;
	};
}

#endif

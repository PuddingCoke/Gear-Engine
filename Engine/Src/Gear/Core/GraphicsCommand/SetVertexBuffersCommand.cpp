#include <Gear/Core/GraphicsCommand/SetVertexBuffersCommand.h>

namespace Gear::Core
{

	void SetVertexBuffersCommand::set(const uint64_t vertexBufferDescsPtrOffsetParam,
	                                  const uint32_t numVertexBufferDescsParam,
	                                  const uint32_t startSlotParam)
	{
		vertexBufferDescsPtrOffset = vertexBufferDescsPtrOffsetParam;

		numVertexBufferDescs = numVertexBufferDescsParam;

		startSlot = startSlotParam;
	}

	void SetVertexBuffersCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setVertexBuffers(vertexBufferDescsPtrOffset, numVertexBufferDescs, startSlot);
	}

}

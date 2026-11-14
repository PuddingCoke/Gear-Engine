#include <Gear/Core/GraphicsCommand/SetVertexBuffersCommand.h>

namespace Gear::Core
{

	void SetVertexBuffersCommand::set(const uint64_t vertexBufferDescsPtrOffset,
	                                  const uint32_t numVertexBufferDescs,
	                                  const uint32_t startSlot)
	{
		this->vertexBufferDescsPtrOffset = vertexBufferDescsPtrOffset;

		this->numVertexBufferDescs = numVertexBufferDescs;

		this->startSlot = startSlot;
	}

	void SetVertexBuffersCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setVertexBuffers(vertexBufferDescsPtrOffset, numVertexBufferDescs, startSlot);
	}

}

#include <Gear/Core/GraphicsCommand/SetVertexBuffersCommand.h>

namespace Gear::Core
{
	SetVertexBuffersCommand::SetVertexBuffersCommand(
		const Resource::VertexBufferDesc* const vertexBufferDescs,
		const uint32_t numVertexBufferDescs,
		const uint32_t startSlot)
	{
		set(vertexBufferDescs, numVertexBufferDescs, startSlot);
	}

	void SetVertexBuffersCommand::set(const Resource::VertexBufferDesc* const vertexBufferDescs,
	                                  const uint32_t numVertexBufferDescs,
	                                  const uint32_t startSlot)
	{
		memcpy(this->vertexBufferDescs,
		       vertexBufferDescs,
		       sizeof(Resource::VertexBufferDesc) * numVertexBufferDescs);

		this->numVertexBufferDescs = numVertexBufferDescs;

		this->startSlot = startSlot;
	}

	void SetVertexBuffersCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setVertexBuffers(vertexBufferDescs, numVertexBufferDescs, startSlot);
	}
}

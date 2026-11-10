#ifndef _GEAR_CORE_SETVERTEXBUFFERSCOMMAND_H_
#define _GEAR_CORE_SETVERTEXBUFFERSCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class SetVertexBuffersCommand : public GraphicsCommand
	{
	public:

		SetVertexBuffersCommand() = delete;

		SetVertexBuffersCommand(const Resource::VertexBufferDesc* const vertexBufferDescs,
		                        const uint32_t numVertexBuffersDescs,
		                        const uint32_t startSlot);

		void set(const Resource::VertexBufferDesc* const vertexBufferDescs,
		         const uint32_t numVertexBuffersDescs,
		         const uint32_t startSlot);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		Resource::VertexBufferDesc vertexBufferDescs[D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

		uint32_t numVertexBufferDescs;

		uint32_t startSlot;
	};
}

#endif

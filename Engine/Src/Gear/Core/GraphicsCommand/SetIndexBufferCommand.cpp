#include<Gear/Core/GraphicsCommand/SetIndexBufferCommand.h>

namespace Gear::Core
{
    SetIndexBufferCommand::SetIndexBufferCommand(const Resource::IndexBufferDesc& indexBufferDesc)
    {
        set(indexBufferDesc);
    }

	void SetIndexBufferCommand::set(const Resource::IndexBufferDesc& indexBufferDesc)
    {
        this->indexBufferDesc = indexBufferDesc;
    }

	void SetIndexBufferCommand::execute(GraphicsCommandExecuter& executer)
    {
        executer.setIndexBuffer(indexBufferDesc);
    }
}
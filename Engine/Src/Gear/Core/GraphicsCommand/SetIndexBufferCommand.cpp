#include <Gear/Core/GraphicsCommand/SetIndexBufferCommand.h>

namespace Gear::Core
{

	void SetIndexBufferCommand::set(const Resource::IndexBufferDesc& indexBufferDesc)
	{
		this->indexBufferDesc = indexBufferDesc;
	}

	void SetIndexBufferCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setIndexBuffer(indexBufferDesc);
	}

}

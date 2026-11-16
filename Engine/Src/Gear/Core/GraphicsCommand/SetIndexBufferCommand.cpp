#include <Gear/Core/GraphicsCommand/SetIndexBufferCommand.h>

namespace Gear::Core
{

	void SetIndexBufferCommand::set(const Resource::IndexBufferDesc& indexBufferDescParam)
	{
		indexBufferDesc = indexBufferDescParam;
	}

	void SetIndexBufferCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setIndexBuffer(indexBufferDesc);
	}

}

#include <Gear/Core/GraphicsCommand/UpdateBufferCommand.h>

namespace Gear::Core
{

	void UpdateBufferCommand::set(D3D12Resource::Buffer* const bufferParam,
	                              D3D12Resource::UploadHeap* const uploadHeapParam,
	                              const uint64_t sizeParam)
	{
		buffer = bufferParam;

		uploadHeap = uploadHeapParam;

		size = sizeParam;
	}

	void UpdateBufferCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.updateBuffer(buffer, uploadHeap, size);
	}

}

#include <Gear/Core/GraphicsCommand/UpdateBufferCommand.h>

namespace Gear::Core
{

	void UpdateBufferCommand::set(D3D12Resource::Buffer* const buffer,
		         D3D12Resource::UploadHeap* const uploadHeap,
		         const uint64_t size)
	{
		this->buffer = buffer;

		this->uploadHeap = uploadHeap;

		this->size = size;
	}

	void UpdateBufferCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.updateBuffer(buffer, uploadHeap, size);
	}

}

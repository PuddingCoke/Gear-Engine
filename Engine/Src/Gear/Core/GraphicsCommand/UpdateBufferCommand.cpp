#include <Gear/Core/GraphicsCommand/UpdateBufferCommand.h>

namespace Gear::Core
{
	UpdateBufferCommand::UpdateBufferCommand(Resource::BufferView& bufferView,
	                                         const void* const data,
	                                         const uint64_t size)
	{
		set(bufferView, data, size);
	}

	UpdateBufferCommand::UpdateBufferCommand(Resource::DefaultCBuffer& defaultCBuffer,
	                                         const void* const data,
	                                         const uint64_t size)
	{
		set(defaultCBuffer, data, size);
	}

	void UpdateBufferCommand::set(Resource::BufferView& bufferView,
	                              const void* const data,
	                              const uint64_t size)
	{
		const Resource::BufferView::UpdateStruct updateStruct = bufferView.getUpdateStruct(data, size);

		this->buffer = updateStruct.buffer;

		this->uploadHeap = updateStruct.uploadHeap;

		this->size = size;
	}

	void UpdateBufferCommand::set(Resource::DefaultCBuffer& defaultCBuffer,
	                              const void* const data,
	                              const uint64_t size)
	{
		const Resource::DefaultCBuffer::UpdateStruct updateStruct =
			defaultCBuffer.getUpdateStruct(data, size);

		this->buffer = updateStruct.buffer;

		this->uploadHeap = updateStruct.uploadHeap;

		this->size = size;
	}

	void UpdateBufferCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.updateBuffer(buffer, uploadHeap, size);
	}
}

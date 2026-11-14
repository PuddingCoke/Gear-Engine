#ifndef _GEAR_CORE_UPDATEBUFFERCOMMAND_H_
#define _GEAR_CORE_UPDATEBUFFERCOMMAND_H_

#include "GraphicsCommand.h"

#include <Gear/Resource/BufferView.h>

#include <Gear/Resource/DefaultCBuffer.h>

namespace Gear::Core
{
	class UpdateBufferCommand : public GraphicsCommand, public CommandPool<UpdateBufferCommand>
	{
	public:

		void set(D3D12Resource::Buffer* const buffer,
		         D3D12Resource::UploadHeap* const uploadHeap,
		         const uint64_t size);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		D3D12Resource::Buffer* buffer;

		D3D12Resource::UploadHeap* uploadHeap;

		uint64_t size;
	};
}

#endif // !_GEAR_CORE_UPDATEBUFFERCOMMAND_H_

#include<Gear/Core/D3D12Core/CommandList.h>

#include<Gear/Core/Graphics.h>

namespace Gear::Core::D3D12Core
{
	CommandList::CommandList(const D3D12_COMMAND_LIST_TYPE type) :
		commandList(nullptr), commandListType(type)
	{
		allocators = makeUnique<ComPtr<ID3D12CommandAllocator>[]>(Graphics::getFrameBufferCount());

		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			CHECKERROR(GraphicsDevice::get()->CreateCommandAllocator(type, IID_PPV_ARGS(&allocators[i])));
		}
	}

	ID3D12CommandList* CommandList::get() const
	{
		return commandList;
	}

	D3D12_COMMAND_LIST_TYPE CommandList::getType() const
	{
		return commandListType;
	}

	void CommandList::flushResourceBarriers()
	{
		flushTransitionResources();

		if (resourceBarriers.size())
		{
			resourceBarrier(static_cast<uint32_t>(resourceBarriers.size()), resourceBarriers.data());

			resourceBarriers.clear();
		}
	}

	void CommandList::setAndCloseCommandList(ID3D12CommandList* const commandList)
	{
		this->commandList = commandList;

		close();
	}

	void CommandList::resetCommandAllocator() const
	{
		allocators[Graphics::getFrameIndex()].Get()->Reset();
	}

	ID3D12CommandAllocator* CommandList::getCommandAllocator() const
	{
		return allocators[Graphics::getFrameIndex()].Get();
	}
}


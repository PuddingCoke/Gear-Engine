#include<Gear/Core/D3D12Core/VideoProcessCommandList.h>

namespace Gear::Core::D3D12Core
{
	VideoProcessCommandList::VideoProcessCommandList() :
		CommandList(D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS)
	{
		CHECKERROR(GraphicsDevice::get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS, getCommandAllocator(), nullptr, IID_PPV_ARGS(&commandList)));

		setAndCloseCommandList(commandList.Get());
	}

	void VideoProcessCommandList::open()
	{
		resetCommandAllocator();

		commandList->Reset(getCommandAllocator());
	}

	void VideoProcessCommandList::close()
	{
		commandList->Close();
	}

	void VideoProcessCommandList::resourceBarrier(const uint32_t numBarriers, const D3D12_RESOURCE_BARRIER* const pBarriers) const
	{
		commandList->ResourceBarrier(numBarriers, pBarriers);
	}
}
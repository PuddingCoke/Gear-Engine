#include <Gear/Core/D3D12Core/VideoProcessCommandList.h>

namespace Gear::Core::D3D12Core
{
	VPInputArguments::VPInputArguments(D3D12Resource::Texture* const texture) : args{}, texture(texture)
	{
		args.InputStream[0].pTexture2D = texture->getResource();
		args.InputStream[0].Subresource = 0;
		args.Transform.SourceRectangle = {
			0, 0, static_cast<LONG>(texture->getWidth()), static_cast<LONG>(texture->getHeight())
		};
		args.Transform.DestinationRectangle = {
			0, 0, static_cast<LONG>(texture->getWidth()), static_cast<LONG>(texture->getHeight())
		};
	}

	VPOutputArguments::VPOutputArguments(D3D12Resource::Texture* const texture) : args{}, texture(texture)
	{
		args.OutputStream[0].pTexture2D = texture->getResource();
		args.OutputStream[0].Subresource = 0;
		args.TargetRectangle = {
			0, 0, static_cast<LONG>(texture->getWidth()), static_cast<LONG>(texture->getHeight())
		};
	}

	VideoProcessCommandList::VideoProcessCommandList() : CommandList(D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS)
	{
		CHECKERROR(GraphicsDevice::get()->CreateCommandList(0,
		                                                    D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS,
		                                                    getCommandAllocator(),
		                                                    nullptr,
		                                                    IID_PPV_ARGS(&commandList)));

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

	void VideoProcessCommandList::resourceBarrier(const uint32_t numBarriers,
	                                              const D3D12_RESOURCE_BARRIER* const pBarriers) const
	{
		commandList->ResourceBarrier(numBarriers, pBarriers);
	}
}

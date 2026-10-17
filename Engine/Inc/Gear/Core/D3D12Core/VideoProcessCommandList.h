#pragma once

#ifndef _GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_
#define _GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_

#include"CommandList.h"

#include<directx/d3d12video.h>

namespace Gear::Core::D3D12Core
{
	struct VPInputArguments
	{

		VPInputArguments(D3D12Resource::Texture* const texture);

		D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS args;

		D3D12Resource::Texture* const texture;

	};

	struct VPOutputArguments
	{

		VPOutputArguments(D3D12Resource::Texture* const texture);

		D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS args;

		D3D12Resource::Texture* const texture;

	};

	CREATESAFETYPE(VideoProcessCommandList);

	class VideoProcessCommandList :public CommandList
	{
	public:

		VideoProcessCommandList();

		~VideoProcessCommandList() = default;

		void open() override;

		void close() override;

		void resourceBarrier(const uint32_t numBarriers, const D3D12_RESOURCE_BARRIER* const pBarriers) const override;

		template<size_t N>
		void processFrames(ID3D12VideoProcessor* const vp, const VPOutputArguments& outputArgs, const VPInputArguments(&inputArgs)[N]);

	private:

		ComPtr<ID3D12VideoProcessCommandList2> commandList;

	};

	template<size_t N>
	inline void VideoProcessCommandList::processFrames(ID3D12VideoProcessor* const vp, const VPOutputArguments& outputArgs, const VPInputArguments(&inputArgs)[N])
	{
		trackAndSetResourceState(outputArgs.texture, D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE);

		for (uint32_t i = 0; i < N; i++)
		{
			trackAndSetResourceState(inputArgs[i].texture, D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ);
		}

		const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS d3d12OutputArgs = outputArgs.args;

		D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS d3d12InputArgs[N] = {};

		for (uint32_t i = 0; i < N; i++)
		{
			d3d12InputArgs[i] = inputArgs[i].args;
		}

		flushResourceBarriers();

		commandList->ProcessFrames(vp, &d3d12OutputArgs, N, d3d12InputArgs);

		trackAndSetResourceState(outputArgs.texture, D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COMMON);

		for (uint32_t i = 0; i < N; i++)
		{
			trackAndSetResourceState(inputArgs[i].texture, D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COMMON);
		}

		flushResourceBarriers();
	}
}

#endif // !_GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_

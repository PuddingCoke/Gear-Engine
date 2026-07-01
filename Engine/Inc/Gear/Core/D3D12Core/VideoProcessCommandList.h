#pragma once

#ifndef _GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_
#define _GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_

#include"CommandList.h"

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(VideoProcessCommandList);

	class VideoProcessCommandList :public CommandList
	{
	public:

		VideoProcessCommandList();

		~VideoProcessCommandList() = default;

		void open() override;

		void close() override;

		void resourceBarrier(const uint32_t numBarriers, const D3D12_RESOURCE_BARRIER* const pBarriers) const override;

	private:

		ComPtr<ID3D12VideoProcessCommandList2> commandList;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_

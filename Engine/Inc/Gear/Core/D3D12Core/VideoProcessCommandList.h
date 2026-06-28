#pragma once

#ifndef _GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_
#define _GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_

namespace Gear::Core::D3D12Core
{
	class VideoProcessCommandList
	{
	public:

		VideoProcessCommandList();

		~VideoProcessCommandList() = default;

	private:

		UniquePtr<ComPtr<ID3D12CommandAllocator>[]> allocators;

		ComPtr<ID3D12GraphicsCommandList6> commandList;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_VIDEOPROCESSCOMMANDLIST_H_

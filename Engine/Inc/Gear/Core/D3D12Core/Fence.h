#pragma once

#ifndef _GEAR_CORE_D3D12CORE_FENCE_H_
#define _GEAR_CORE_D3D12CORE_FENCE_H_

#include <Gear/Core/GraphicsDevice.h>

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(Fence);

	class Fence
	{
	public:

		Fence(const D3D12_FENCE_FLAGS flags = D3D12_FENCE_FLAG_NONE);

		~Fence();

		ID3D12Fence* get() const;

		//返回自增后的值
		//注意：仅用于内部使用和第三方库
		uint64_t increment();

		//CPU等待
		void waitValue(const uint64_t value);

		uint64_t signal(ID3D12CommandQueue* const commandQueue);

		void wait(ID3D12CommandQueue* const commandQueue, const uint64_t waitValue);

		uint64_t getCompletedValue() const;

	private:

		ComPtr<ID3D12Fence> fence;

		HANDLE fenceEvent;

		std::atomic<uint64_t> currentFenceValue;
	};
}

#endif // !_GEAR_CORE_D3D12CORE_FENCE_H_

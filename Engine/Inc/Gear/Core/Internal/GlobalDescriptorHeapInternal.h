#pragma once

#ifndef _GEAR_CORE_GLOBALDESCRIPTORHEAP_INTERNAL_H_
#define _GEAR_CORE_GLOBALDESCRIPTORHEAP_INTERNAL_H_

#include <cstdint>

namespace Gear::Core::GlobalDescriptorHeap::Internal
{
	void initialize();

	void release();

	struct InitializeToken
	{
		InitializeToken()
		{
			initialize();
		}
		~InitializeToken()
		{
			release();
		}
	};

	//官方文档的推荐上限
	constexpr uint32_t numResourceHeapDescriptors = 1000000u;

	//引擎内部的快速算法和旧第三方API需要连续的资源描述符
	constexpr uint32_t numContiguousResourceHeapDescriptors = 16384u;

	//官方文档的推荐上限
	constexpr uint32_t numSamplerHeapDescriptors = 1024u;

	//目前只有ClearUnorderedAccessView需要，不知道有没有其它的用途需要，所以先留些空间
	constexpr uint32_t numNonShaderVisibleResourceHeapDescriptors = 65536u;

	constexpr uint32_t numRTVDescriptors = 16384u;

	//旧第三方API需要连续的RTV/DSV描述符
	constexpr uint32_t numContiguousRTVDescriptors = 2048u;
}

#endif // !_GEAR_CORE_GLOBALDESCRIPTORHEAP_INTERNAL_H_

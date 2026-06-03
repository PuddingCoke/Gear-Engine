#pragma once

#ifndef _GEAR_CORE_GLOBALDESCRIPTORHEAP_INTERNAL_H_
#define _GEAR_CORE_GLOBALDESCRIPTORHEAP_INTERNAL_H_

#include<cstdint>

namespace Gear::Core::GlobalDescriptorHeap::Internal
{
	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

	constexpr uint32_t numResourceHeapDescriptors = 1000000u;

	constexpr uint32_t numStaticCBVSRVUAVDescriptors = 200000u;
}

#endif // !_GEAR_CORE_GLOBALDESCRIPTORHEAP_INTERNAL_H_

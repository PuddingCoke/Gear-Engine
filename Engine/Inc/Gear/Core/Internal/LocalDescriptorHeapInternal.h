#pragma once

#ifndef _GEAR_CORE_LOCALDESCRIPTORHEAP_INTERNAL_H_
#define _GEAR_CORE_LOCALDESCRIPTORHEAP_INTERNAL_H_

#include<cstdint>

namespace Gear::Core::LocalDescriptorHeap::Internal
{
	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

	constexpr uint32_t numStagingResourceDescriptors = 2000000u;

	constexpr uint32_t numStaticSRVDescriptors = 100000u;

	constexpr uint32_t numRTVDescriptors = 500000u;

	constexpr uint32_t numStaticRTVDescriptors = 150000u;
}

#endif // !_GEAR_CORE_LOCALDESCRIPTORHEAP_INTERNAL_H_

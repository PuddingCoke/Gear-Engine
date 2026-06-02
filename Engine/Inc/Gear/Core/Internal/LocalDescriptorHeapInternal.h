#pragma once

#ifndef _GEAR_CORE_LOCALDESCRIPTORHEAP_INTERNAL_H_
#define _GEAR_CORE_LOCALDESCRIPTORHEAP_INTERNAL_H_

namespace Gear
{
	namespace Core
	{
		namespace LocalDescriptorHeap
		{
			namespace Internal
			{

				void initialize();

				void release();

				constexpr uint32_t numStagingResourceDescriptors = 2000000u;

				constexpr uint32_t numStaticSRVDescriptors = 100000u;

				constexpr uint32_t numRTVDescriptors = 500000u;

				constexpr uint32_t numStaticRTVDescriptors = 150000u;

			}
		}
	}
}

#endif // !_GEAR_CORE_LOCALDESCRIPTORHEAP_INTERNAL_H_

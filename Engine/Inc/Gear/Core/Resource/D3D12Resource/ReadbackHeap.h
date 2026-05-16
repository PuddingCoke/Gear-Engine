#pragma once

#ifndef _GEAR_CORE_RESOURCE_D3D12RESOURCE_READBACKHEAP_H_
#define _GEAR_CORE_RESOURCE_D3D12RESOURCE_READBACKHEAP_H_

#include"D3D12ResourceBase.h"

namespace Gear
{
	namespace Core
	{
		namespace Resource
		{
			namespace D3D12Resource
			{
				class ReadbackHeap :public D3D12ResourceBase
				{
				public:

					ReadbackHeap() = delete;

					ReadbackHeap(const ReadbackHeap&) = delete;

					void operator=(const ReadbackHeap&) = delete;

					ReadbackHeap(const uint64_t size, const D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE);

					ReadbackHeap(ReadbackHeap&);

					virtual ~ReadbackHeap();

					void* map(const D3D12_RANGE readRange) const;

					void unmap() const;

					void updateGlobalStates() override;

					void resetInternalStates() override;

				protected:

					void resetTransitionStates() override;

				};
			}
		}
	}
}

#endif // !_GEAR_CORE_RESOURCE_D3D12RESOURCE_READBACKHEAP_H_
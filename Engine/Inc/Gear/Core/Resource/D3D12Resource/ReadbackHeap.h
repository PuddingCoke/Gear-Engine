#pragma once

#ifndef _GEAR_CORE_RESOURCE_D3D12RESOURCE_READBACKHEAP_H_
#define _GEAR_CORE_RESOURCE_D3D12RESOURCE_READBACKHEAP_H_

#include"D3D12ResourceBase.h"

namespace Gear::Core::Resource::D3D12Resource
{
	CREATESAFETYPE(ReadbackHeap);

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

	};
}

#endif // !_GEAR_CORE_RESOURCE_D3D12RESOURCE_READBACKHEAP_H_
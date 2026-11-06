#pragma once

#ifndef _GEAR_CORE_D3D12RESOURCE_READBACKHEAP_H_
#define _GEAR_CORE_D3D12RESOURCE_READBACKHEAP_H_

#include "D3D12ResourceBase.h"

namespace Gear::Core::D3D12Resource
{
	CREATESAFETYPE(ReadbackHeap);

	class ReadbackHeap : public D3D12ResourceBase
	{
	public:

		ReadbackHeap() = delete;

		ReadbackHeap(const ReadbackHeap&) = delete;

		void operator=(const ReadbackHeap&) = delete;

		ReadbackHeap(const uint64_t size, const D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE);

		ReadbackHeap(ReadbackHeap&);

		virtual ~ReadbackHeap();

		const void* map(const uint64_t begin, const uint64_t end) const;

		const void* map() const;

		void unmap() const;

		uint64_t getSize() const;

	private:

		const uint64_t size;
	};
}

#endif // !_GEAR_CORE_D3D12RESOURCE_READBACKHEAP_H_

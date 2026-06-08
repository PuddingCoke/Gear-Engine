#pragma once

#ifndef _GEAR_CORE_RESOURCE_D3D12RESOURCE_UPLOADHEAP_H_
#define _GEAR_CORE_RESOURCE_D3D12RESOURCE_UPLOADHEAP_H_

#include"D3D12ResourceBase.h"

namespace Gear::Core::Resource::D3D12Resource
{
	CREATESAFETYPE(UploadHeap);

	class UploadHeap :public D3D12ResourceBase
	{
	public:

		UploadHeap() = delete;

		UploadHeap(const UploadHeap&) = delete;

		void operator=(const UploadHeap&) = delete;

		UploadHeap(const uint64_t size, const D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE);

		UploadHeap(UploadHeap&);

		virtual ~UploadHeap();

		void* map() const;

		void unmap() const;

		void update(const void* const data, const uint64_t size) const;

		void updateGlobalStates() override;

		void resetInternalStates() override;

	protected:

		void resetTransitionStates() override;

	};
}

#endif // !_GEAR_CORE_RESOURCE_D3D12RESOURCE_UPLOADHEAP_H_
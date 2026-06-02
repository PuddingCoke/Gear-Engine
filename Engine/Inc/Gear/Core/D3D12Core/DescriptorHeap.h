#pragma once

#ifndef _GEAR_CORE_D3D12CORE_DESCRIPTORHEAP_H_
#define _GEAR_CORE_D3D12CORE_DESCRIPTORHEAP_H_

#include<Gear/Core/GraphicsDevice.h>

#include<atomic>

#include"DescriptorHandle.h"

namespace Gear::Core::D3D12Core
{
	class DescriptorHeap
	{
	public:

		DescriptorHeap() = delete;

		DescriptorHeap(const DescriptorHeap&) = delete;

		void operator=(const DescriptorHeap&) = delete;

		DescriptorHeap(const uint32_t numDescriptors, const uint32_t numDynamicDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE type, const D3D12_DESCRIPTOR_HEAP_FLAGS flags);

		ID3D12DescriptorHeap* get() const;

		DescriptorHandle allocStaticDescriptor(const uint32_t num);

		DescriptorHandle allocDynamicDescriptor(const uint32_t num);

	private:

		const uint32_t numDescriptors;

		const uint32_t numDynamicDescriptors;

		const D3D12_DESCRIPTOR_HEAP_TYPE type;

		const uint32_t incrementSize;

		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStart;

		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStart;

		std::atomic<uint64_t> staticIndex;

		std::atomic<uint64_t> dynamicIndex;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_DESCRIPTORHEAP_H_
#pragma once

#ifndef _GEAR_CORE_D3D12CORE_DESCRIPTORHEAP_H_
#define _GEAR_CORE_D3D12CORE_DESCRIPTORHEAP_H_

#include <Gear/Core/GraphicsDevice.h>

#include <mutex>

#include <vector>

#include "DescriptorHandle.h"

namespace Gear::Core::D3D12Core
{
	class DescriptorHeap
	{
	public:

		DescriptorHeap() = delete;

		DescriptorHeap(const DescriptorHeap&) = delete;

		void operator=(const DescriptorHeap&) = delete;

		DescriptorHeap(const uint32_t numDescriptorsParam,
		               const uint32_t numContiguousDescriptorsParam,
		               const D3D12_DESCRIPTOR_HEAP_TYPE typeParam,
		               const D3D12_DESCRIPTOR_HEAP_FLAGS flagsParam);

		ID3D12DescriptorHeap* get() const;

		//请尽可能使用无序描述符分配，因为后者可分配范围大得多且性能最好，连续描述符仅用于引擎内部的快速实现和兼容少数旧的模块
		UniquePtr<DescriptorHandle> allocContiguousDescriptor(const uint32_t num);

		UniquePtr<DescriptorHandle> allocUnorderedDescriptor(const uint32_t num);

		void freeContiguousDescriptor(const uint32_t start, const uint32_t count);

		void freeUnorderedDescriptor(const std::vector<uint32_t>& indices);

		uint32_t getNumDescriptors() const;

		uint32_t getNumContiguousDescriptors() const;

		uint32_t getNumUnorderedDescriptors() const;

	private:

		const uint32_t numDescriptors;

		const uint32_t numContiguousDescriptors;

		const uint32_t numUnorderedDescriptors;

		const D3D12_DESCRIPTOR_HEAP_TYPE type;

		const uint32_t incrementSize;

		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStart;

		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStart;

		std::mutex contiguousMutex;

		struct Run
		{
			uint32_t start;

			uint32_t count;
		};

		std::vector<Run> runs;

		std::mutex unorderedMutex;

		std::vector<uint32_t> unorderedIndices;
	};

}

#endif // !_GEAR_CORE_D3D12CORE_DESCRIPTORHEAP_H_

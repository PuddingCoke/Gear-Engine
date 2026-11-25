#pragma once

#ifndef _GEAR_CORE_D3D12CORE_DESCRIPTORHANDLE_H_
#define _GEAR_CORE_D3D12CORE_DESCRIPTORHANDLE_H_

#include <Gear/Core/GraphicsDevice.h>

namespace Gear::Core::D3D12Core
{
	class DescriptorHeap;

	CREATESAFETYPE(DescriptorHandle);

	class DescriptorHandle
	{
	public:

		DescriptorHandle(DescriptorHeap* const descriptorHeapParam,
		                 const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStartParam,
		                 const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStartParam,
		                 const uint32_t baseOffsetParam,
		                 const uint32_t incrementSizeParam,
		                 const uint32_t numDescriptorsParam);

		virtual ~DescriptorHandle() = default;

		CD3DX12_CPU_DESCRIPTOR_HANDLE getCurrentCPUHandle() const;

		CD3DX12_GPU_DESCRIPTOR_HANDLE getCurrentGPUHandle() const;

		virtual uint32_t getCurrentIndex() const = 0;

		uint32_t getNumDescriptors() const;

		DescriptorHeap* getDescriptorHeap() const;

		void move(const uint32_t num = 1u);

		void reset();

	protected:

		uint32_t getCurrentOffset() const;

		uint32_t getBaseOffset() const;

		uint32_t getIncrementSize() const;

	private:

		DescriptorHeap* const descriptorHeap;

		//指向堆开头的CPU句柄
		const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStart;

		//指向堆开头的GPU句柄
		const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStart;

		//基础偏移
		const uint32_t baseOffset;

		//描述符增量
		const uint32_t incrementSize;

		//描述符数量
		const uint32_t numDescriptors;

		//基于baseOffset的偏移
		uint32_t currentOffset;
	};
}

#endif // !_GEAR_CORE_D3D12CORE_DESCRIPTORHANDLE_H_

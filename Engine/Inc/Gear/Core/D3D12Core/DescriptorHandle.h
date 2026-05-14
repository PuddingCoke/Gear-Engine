#pragma once

#ifndef _GEAR_CORE_D3D12CORE_DESCRIPTORHANDLE_H_
#define _GEAR_CORE_D3D12CORE_DESCRIPTORHANDLE_H_

#include<Gear/Core/GraphicsDevice.h>

namespace Gear
{
	namespace Core
	{
		namespace D3D12Core
		{
			class DescriptorHeap;

			class DescriptorHandle
			{
			public:

				DescriptorHandle();

				DescriptorHandle(const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStart, const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStart,
					const uint32_t currentOffset, const uint32_t incrementSize, const uint32_t baseIndex, const uint32_t offsetLimit, const DescriptorHeap* descriptorHeap);

				CD3DX12_CPU_DESCRIPTOR_HANDLE getCurrentCPUHandle() const;

				CD3DX12_GPU_DESCRIPTOR_HANDLE getCurrentGPUHandle() const;

				uint32_t getCurrentIndex() const;

				uint32_t getIncrementSize() const;

				//到堆尾的长度（环绕前长度）
				uint32_t getDistToEnd() const;

				const DescriptorHeap* getDescriptorHeap() const;

				void move(const uint32_t num = 1u);

				//它会进行适用于环形情况的拷贝
				static void copyDescriptors(const uint32_t numCopy, DescriptorHandle copyDestHandle, DescriptorHandle copySrcHandle, const D3D12_DESCRIPTOR_HEAP_TYPE heapType);

			protected:

				//它会进行真正的拷贝并移动描述符句柄
				//要在拷贝前要确保拷贝目标和拷贝源在numCopy的跨度内无环绕，要不然会有致命错误
				static void copyDescriptorsSimple(const uint32_t numCopy, DescriptorHandle& copyDestHandle, DescriptorHandle& copySrcHandle, const D3D12_DESCRIPTOR_HEAP_TYPE heapType);

				const DescriptorHeap* descriptorHeap;

				//指向堆开头的CPU句柄
				CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStart;

				//指向堆开头的GPU句柄
				CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStart;

				//基于baseIndex的偏移
				uint32_t currentOffset;

				//描述符增量
				uint32_t incrementSize;

				//基础偏移
				uint32_t baseIndex;

				//最大偏移
				uint32_t offsetLimit;

			};
		}
	}
}

#endif // !_GEAR_CORE_D3D12CORE_DESCRIPTORHANDLE_H_

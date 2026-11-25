#include <Gear/Core/D3D12Core/DescriptorHandle.h>

namespace Gear::Core::D3D12Core
{

	DescriptorHandle::DescriptorHandle(DescriptorHeap* const descriptorHeapParam,
	                                   const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStartParam,
	                                   const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStartParam,
	                                   const uint32_t baseOffsetParam,
	                                   const uint32_t incrementSizeParam,
	                                   const uint32_t numDescriptorsParam) :
		descriptorHeap(descriptorHeapParam),
		cpuHandleStart(cpuHandleStartParam),
		gpuHandleStart(gpuHandleStartParam),
		baseOffset(baseOffsetParam),
		incrementSize(incrementSizeParam),
		numDescriptors(numDescriptorsParam),
		currentOffset(baseOffsetParam)
	{
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHandle::getCurrentCPUHandle() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandleStart, getCurrentIndex(), incrementSize);
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHandle::getCurrentGPUHandle() const
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuHandleStart, getCurrentIndex(), incrementSize);
	}

	uint32_t DescriptorHandle::getNumDescriptors() const
	{
		return numDescriptors;
	}

	DescriptorHeap* DescriptorHandle::getDescriptorHeap() const
	{
		return descriptorHeap;
	}

	void DescriptorHandle::move(const uint32_t num)
	{
		currentOffset += num;
	}

	void DescriptorHandle::reset()
	{
		currentOffset = baseOffset;
	}

	uint32_t DescriptorHandle::getCurrentOffset() const
	{
#ifdef _DEBUG
		if (currentOffset - baseOffset >= numDescriptors)
		{
			THROWLOG(LOGERROR() << "偏移越界！");
		}
#endif

		return currentOffset;
	}

	uint32_t DescriptorHandle::getBaseOffset() const
	{
		return baseOffset;
	}

	uint32_t DescriptorHandle::getIncrementSize() const
	{
		return incrementSize;
	}

}

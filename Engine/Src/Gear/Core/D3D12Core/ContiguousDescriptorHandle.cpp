#include <Gear/Core/D3D12Core/ContiguousDescriptorHandle.h>

#include <Gear/Core/D3D12Core/DescriptorHeap.h>

namespace Gear::Core::D3D12Core
{
	ContiguousDescriptorHandle::ContiguousDescriptorHandle(
		DescriptorHeap* const descriptorHeapParam,
		const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStartParam,
		const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStartParam,
		const uint32_t baseOffsetParam,
		const uint32_t incrementSizeParam,
		const uint32_t numDescriptorsParam) :
		DescriptorHandle(descriptorHeapParam,
	                     cpuHandleStartParam,
	                     gpuHandleStartParam,
	                     baseOffsetParam,
	                     incrementSizeParam,
	                     numDescriptorsParam)
	{
	}

	ContiguousDescriptorHandle::~ContiguousDescriptorHandle()
	{
		getDescriptorHeap()->freeContiguousDescriptor(getBaseOffset(), getNumDescriptors());
	}

	uint32_t ContiguousDescriptorHandle::getCurrentIndex() const
	{
		return getCurrentOffset();
	}
}

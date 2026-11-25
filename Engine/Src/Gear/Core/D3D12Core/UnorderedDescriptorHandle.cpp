#include <Gear/Core/D3D12Core/UnorderedDescriptorHandle.h>

#include <Gear/Core/D3D12Core/DescriptorHeap.h>

namespace Gear::Core::D3D12Core
{
	UnorderedDescriptorHandle::UnorderedDescriptorHandle(
		DescriptorHeap* const descriptorHeapParam,
		const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStartParam,
		const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStartParam,
		const uint32_t incrementSizeParam,
		const std::vector<uint32_t>& descriptorIndicesParam) :
		DescriptorHandle(descriptorHeapParam,
	                     cpuHandleStartParam,
	                     gpuHandleStartParam,
	                     0u,
	                     incrementSizeParam,
	                     static_cast<uint32_t>(descriptorIndicesParam.size())),
		descriptorIndices(descriptorIndicesParam)
	{
	}

	UnorderedDescriptorHandle::~UnorderedDescriptorHandle()
	{
		getDescriptorHeap()->freeUnorderedDescriptor(descriptorIndices);
	}

	uint32_t UnorderedDescriptorHandle::getCurrentIndex() const
	{
		return descriptorIndices[getCurrentOffset()];
	}
}

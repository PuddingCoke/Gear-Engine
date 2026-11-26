#ifndef _GEAR_CORE_D3D12CORE_UNORDEREDDESCRIPTORHANDLE_H_
#define _GEAR_CORE_D3D12CORE_UNORDEREDDESCRIPTORHANDLE_H_

#include "Gear/Core/D3D12Core/DescriptorHandle.h"

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(UnorderedDescriptorHandle);

	class UnorderedDescriptorHandle : public DescriptorHandle
	{
	public:

		UnorderedDescriptorHandle(DescriptorHeap* const descriptorHeapParam,
		                          const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStartParam,
		                          const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStartParam,
		                          const uint32_t incrementSizeParam,
		                          const std::vector<uint32_t>& descriptorIndicesParam);

		UnorderedDescriptorHandle(const UnorderedDescriptorHandle&) = delete;

		void operator=(const UnorderedDescriptorHandle&) = delete;

		~UnorderedDescriptorHandle();

		uint32_t getCurrentIndex() const override;

	private:

		const std::vector<uint32_t> descriptorIndices;
	};
}

#endif // _GEAR_CORE_D3D12CORE_UNORDEREDDESCRIPTORHANDLE_H_

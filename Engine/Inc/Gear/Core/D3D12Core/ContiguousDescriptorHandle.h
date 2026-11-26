#ifndef _GEAR_CORE_D3D12CORE_CONTIGUOUSDESCRIPTORHANDLE_H_
#define _GEAR_CORE_D3D12CORE_CONTIGUOUSDESCRIPTORHANDLE_H_

#include "DescriptorHandle.h"

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(ContiguousDescriptorHandle);

	class ContiguousDescriptorHandle : public DescriptorHandle
	{
	public:

		ContiguousDescriptorHandle(DescriptorHeap* const descriptorHeapParam,
		                           const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStartParam,
		                           const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStartParam,
		                           const uint32_t baseOffsetParam,
		                           const uint32_t incrementSizeParam,
		                           const uint32_t numDescriptorsParam);

		ContiguousDescriptorHandle(const ContiguousDescriptorHandle&) = delete;

		void operator=(const ContiguousDescriptorHandle&) = delete;

		~ContiguousDescriptorHandle();

		uint32_t getCurrentIndex() const override;
	};
}

#endif // !_GEAR_CORE_D3D12CORE_CONTIGUOUSDESCRIPTORHANDLE_H_

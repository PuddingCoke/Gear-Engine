#include<Gear/Core/GlobalDescriptorHeap.h>

#include<Gear/Core/Internal/GlobalDescriptorHeapInternal.h>

namespace
{
	struct GlobalDescriptorHeapImpl
	{

		UniquePtr<Gear::Core::D3D12Core::DescriptorHeap> resourceHeap;

		UniquePtr<Gear::Core::D3D12Core::DescriptorHeap> samplerHeap;

	}impl;
}

void Gear::Core::GlobalDescriptorHeap::Internal::initialize()
{
	impl.resourceHeap = makeUnique<D3D12Core::DescriptorHeap>(numResourceHeapDescriptors, numResourceHeapDescriptors - numStaticCBVSRVUAVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	impl.samplerHeap = makeUnique<D3D12Core::DescriptorHeap>(1024, 0, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"global descriptor heaps", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalDescriptorHeap::Internal::release()
{
	impl.resourceHeap.reset();

	impl.samplerHeap.reset();
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::GlobalDescriptorHeap::getResourceHeap()
{
	return impl.resourceHeap.get();
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::GlobalDescriptorHeap::getSamplerHeap()
{
	return impl.samplerHeap.get();
}


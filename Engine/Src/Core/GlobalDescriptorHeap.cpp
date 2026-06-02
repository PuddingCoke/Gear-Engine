#include<Gear/Core/GlobalDescriptorHeap.h>

#include<Gear/Core/Internal/GlobalDescriptorHeapInternal.h>

namespace
{
	struct GlobalDescriptorHeapImpl
	{

		Gear::Core::D3D12Core::DescriptorHeap* resourceHeap = nullptr;

		Gear::Core::D3D12Core::DescriptorHeap* samplerHeap = nullptr;

	}impl;

	thread_local Gear::Core::D3D12Core::DescriptorHeap* perThreadStagingResourceHeap = nullptr;

	thread_local Gear::Core::D3D12Core::DescriptorHeap* perThreadRenderTargetHeap = nullptr;

	thread_local Gear::Core::D3D12Core::DescriptorHeap* perThreadDepthStencilHeap = nullptr;

}

void Gear::Core::GlobalDescriptorHeap::Internal::initializeGlobalDescriptorHeaps()
{
	impl.resourceHeap = new D3D12Core::DescriptorHeap(numResourceHeapDescriptors, numResourceHeapDescriptors - numStaticCBVSRVUAVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	impl.samplerHeap = new D3D12Core::DescriptorHeap(1024, 0, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"global descriptor heaps", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalDescriptorHeap::Internal::initializeLocalDescriptorHeaps()
{
	perThreadStagingResourceHeap = new D3D12Core::DescriptorHeap(numStagingResourceDescriptors + numStaticSRVDescriptors, numStagingResourceDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	perThreadRenderTargetHeap = new D3D12Core::DescriptorHeap(numRTVDescriptors, numRTVDescriptors - numStaticRTVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	perThreadDepthStencilHeap = new D3D12Core::DescriptorHeap(numRTVDescriptors, numRTVDescriptors - numStaticRTVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"local descriptor heaps", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalDescriptorHeap::Internal::releaseGlobalDescriptorHeaps()
{
	if (impl.resourceHeap)
	{
		delete impl.resourceHeap;
	}

	if (impl.samplerHeap)
	{
		delete impl.samplerHeap;
	}
}

void Gear::Core::GlobalDescriptorHeap::Internal::releaseLocalDescriptorHeaps()
{
	if (perThreadStagingResourceHeap)
	{
		delete perThreadStagingResourceHeap;
	}

	if (perThreadRenderTargetHeap)
	{
		delete perThreadRenderTargetHeap;
	}

	if (perThreadDepthStencilHeap)
	{
		delete perThreadDepthStencilHeap;
	}
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::GlobalDescriptorHeap::getResourceHeap()
{
	return impl.resourceHeap;
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::GlobalDescriptorHeap::getSamplerHeap()
{
	return impl.samplerHeap;
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::GlobalDescriptorHeap::getStagingResourceHeap()
{
#ifdef _DEBUG
	if (!perThreadStagingResourceHeap)
	{
		LOGERROR(L"you haven't allocate per thread descriptor heaps yet!");
	}
#endif // _DEBUG

	return perThreadStagingResourceHeap;
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::GlobalDescriptorHeap::getRenderTargetHeap()
{
#ifdef _DEBUG
	if (!perThreadRenderTargetHeap)
	{
		LOGERROR(L"you haven't allocate per thread descriptor heaps yet!");
	}
#endif // _DEBUG

	return perThreadRenderTargetHeap;
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::GlobalDescriptorHeap::getDepthStencilHeap()
{
#ifdef _DEBUG
	if (!perThreadDepthStencilHeap)
	{
		LOGERROR(L"you haven't allocate per thread descriptor heaps yet!");
	}
#endif // _DEBUG

	return perThreadDepthStencilHeap;
}


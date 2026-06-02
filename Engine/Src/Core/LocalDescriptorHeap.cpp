#include<Gear/Core/LocalDescriptorHeap.h>

#include<Gear/Core/Internal/LocalDescriptorHeapInternal.h>

/// <summary>
/// 之前写多线程日志系统的时候学习了下thread_local
/// 然后突发奇想，想到每个渲染线程其实可以有独享的暂存资源堆、渲染目标堆、深度模板堆
/// 但是这要求执行RenderTask的构造方法的线程和执行workerLoop的线程必须是同一线程
/// </summary>
namespace Gear::Core::LocalDescriptorHeap::Internal
{
	thread_local UniquePtr<D3D12Core::DescriptorHeap> perThreadStagingResourceHeap;

	thread_local UniquePtr<D3D12Core::DescriptorHeap> perThreadRenderTargetHeap;

	thread_local UniquePtr<D3D12Core::DescriptorHeap> perThreadDepthStencilHeap;
}

void Gear::Core::LocalDescriptorHeap::Internal::initialize()
{
	perThreadStagingResourceHeap = makeUnique<D3D12Core::DescriptorHeap>(numStagingResourceDescriptors + numStaticSRVDescriptors, numStagingResourceDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	perThreadRenderTargetHeap = makeUnique<D3D12Core::DescriptorHeap>(numRTVDescriptors, numRTVDescriptors - numStaticRTVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	perThreadDepthStencilHeap = makeUnique<D3D12Core::DescriptorHeap>(numRTVDescriptors, numRTVDescriptors - numStaticRTVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"local descriptor heaps", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::LocalDescriptorHeap::Internal::release()
{
	perThreadStagingResourceHeap.reset();

	perThreadRenderTargetHeap.reset();

	perThreadDepthStencilHeap.reset();
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::LocalDescriptorHeap::getStagingResourceHeap()
{
#ifdef _DEBUG
	if (!Internal::perThreadStagingResourceHeap.get())
	{
		LOGERROR(L"你还没有分配线程独享的描述符堆!");
	}
#endif // _DEBUG

	return Internal::perThreadStagingResourceHeap.get();
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::LocalDescriptorHeap::getRenderTargetHeap()
{
#ifdef _DEBUG
	if (!Internal::perThreadRenderTargetHeap.get())
	{
		LOGERROR(L"你还没有分配线程独享的描述符堆!");
	}
#endif // _DEBUG

	return Internal::perThreadRenderTargetHeap.get();
}

Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::LocalDescriptorHeap::getDepthStencilHeap()
{
#ifdef _DEBUG
	if (!Internal::perThreadDepthStencilHeap.get())
	{
		LOGERROR(L"你还没有分配线程独享的描述符堆!");
	}
#endif // _DEBUG

	return Internal::perThreadDepthStencilHeap.get();
}
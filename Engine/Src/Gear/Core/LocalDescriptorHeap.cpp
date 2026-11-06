#include <Gear/Core/LocalDescriptorHeap.h>

#include <Gear/Core/Internal/LocalDescriptorHeapInternal.h>

/// <summary>
/// 之前写多线程日志系统的时候学习了下thread_local
/// 然后突发奇想，想到每个渲染线程其实可以有独享的暂存资源堆、渲染目标堆、深度模板堆
/// 但是这要求执行RenderTask的构造方法的线程和执行workerLoop的线程必须是同一线程
/// </summary>
namespace Gear::Core::LocalDescriptorHeap
{
	struct LocalDescriptorHeapImpl
	{
		LocalDescriptorHeapImpl();

		UniquePtr<D3D12Core::DescriptorHeap> stagingResourceHeap;

		UniquePtr<D3D12Core::DescriptorHeap> renderTargetHeap;

		UniquePtr<D3D12Core::DescriptorHeap> depthStencilHeap;
	};

	LocalDescriptorHeapImpl::LocalDescriptorHeapImpl()
	{
		stagingResourceHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numStagingResourceDescriptors +
		                                                                Internal::numStaticSRVDescriptors,
		                                                            Internal::numStagingResourceDescriptors,
		                                                            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		                                                            D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		renderTargetHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numRTVDescriptors,
		                                                         Internal::numRTVDescriptors -
		                                                             Internal::numStaticRTVDescriptors,
		                                                         D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		                                                         D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		depthStencilHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numRTVDescriptors,
		                                                         Internal::numRTVDescriptors -
		                                                             Internal::numStaticRTVDescriptors,
		                                                         D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		                                                         D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		LOGSUCCESS() << "创建" << LogColor::brightMagenta << TOSTRING(LocalDescriptorHeap);
	}

	thread_local UniquePtr<LocalDescriptorHeapImpl> impl;

	namespace Internal
	{
		void initialize()
		{
			impl = makeUnique<LocalDescriptorHeapImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	D3D12Core::DescriptorHeap* getStagingResourceHeap()
	{
#ifdef _DEBUG
		if (!impl.get())
		{
			THROWLOG(LOGERROR() << "你还没有申请线程局部描述符堆!");
		}
#endif // _DEBUG

		return impl->stagingResourceHeap.get();
	}

	D3D12Core::DescriptorHeap* getRenderTargetHeap()
	{
#ifdef _DEBUG
		if (!impl.get())
		{
			THROWLOG(LOGERROR() << "你还没有申请线程局部描述符堆!");
		}
#endif // _DEBUG

		return impl->renderTargetHeap.get();
	}

	D3D12Core::DescriptorHeap* getDepthStencilHeap()
	{
#ifdef _DEBUG
		if (!impl.get())
		{
			THROWLOG(LOGERROR() << "你还没有申请线程局部描述符堆!");
		}
#endif // _DEBUG

		return impl->depthStencilHeap.get();
	}
}

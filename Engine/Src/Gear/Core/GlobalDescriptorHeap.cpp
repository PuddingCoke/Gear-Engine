#include <Gear/Core/GlobalDescriptorHeap.h>

#include <Gear/Core/Internal/GlobalDescriptorHeapInternal.h>

namespace Gear::Core::GlobalDescriptorHeap
{
	struct GlobalDescriptorHeapImpl
	{
		GlobalDescriptorHeapImpl();

		UniquePtr<D3D12Core::DescriptorHeap> resourceHeap;

		UniquePtr<D3D12Core::DescriptorHeap> samplerHeap;

		UniquePtr<D3D12Core::DescriptorHeap> nonShaderVisibleResourceHeap;

		UniquePtr<D3D12Core::DescriptorHeap> renderTargetHeap;

		UniquePtr<D3D12Core::DescriptorHeap> depthStencilHeap;
	};

	GlobalDescriptorHeapImpl::GlobalDescriptorHeapImpl()
	{
		resourceHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numResourceHeapDescriptors,
		                                                     Internal::numContiguousResourceHeapDescriptors,
		                                                     D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		                                                     D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		samplerHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numSamplerHeapDescriptors,
		                                                    0,
		                                                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		                                                    D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		nonShaderVisibleResourceHeap =
			makeUnique<D3D12Core::DescriptorHeap>(Internal::numNonShaderVisibleResourceHeapDescriptors,
		                                          0,
		                                          D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		                                          D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		renderTargetHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numRTVDescriptors,
		                                                         Internal::numContiguousRTVDescriptors,
		                                                         D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		                                                         D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		depthStencilHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numRTVDescriptors,
		                                                         Internal::numContiguousRTVDescriptors,
		                                                         D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		                                                         D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		LOGSUCCESS() << "创建" << COLORIZESTRUCT(GlobalDescriptorHeap);
	}

	UniquePtr<GlobalDescriptorHeapImpl> impl;

	namespace Internal
	{
		void initialize()
		{
			impl = makeUnique<GlobalDescriptorHeapImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	D3D12Core::DescriptorHeap* getResourceHeap()
	{
		return impl->resourceHeap.get();
	}

	D3D12Core::DescriptorHeap* getSamplerHeap()
	{
		return impl->samplerHeap.get();
	}

	D3D12Core::DescriptorHeap* getNonShaderVisibleResourceHeap()
	{
		return impl->nonShaderVisibleResourceHeap.get();
	}

	D3D12Core::DescriptorHeap* getRenderTargetHeap()
	{
		return impl->renderTargetHeap.get();
	}

	D3D12Core::DescriptorHeap* getDepthStencilHeap()
	{
		return impl->depthStencilHeap.get();
	}
}

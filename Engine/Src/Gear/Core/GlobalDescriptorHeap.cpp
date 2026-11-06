#include <Gear/Core/GlobalDescriptorHeap.h>

#include <Gear/Core/Internal/GlobalDescriptorHeapInternal.h>

namespace Gear::Core::GlobalDescriptorHeap
{
	struct GlobalDescriptorHeapImpl
	{
		GlobalDescriptorHeapImpl();

		UniquePtr<D3D12Core::DescriptorHeap> resourceHeap;

		UniquePtr<D3D12Core::DescriptorHeap> samplerHeap;
	};

	GlobalDescriptorHeapImpl::GlobalDescriptorHeapImpl()
	{
		resourceHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numResourceHeapDescriptors,
		                                                     Internal::numResourceHeapDescriptors -
		                                                         Internal::numStaticCBVSRVUAVDescriptors,
		                                                     D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		                                                     D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		samplerHeap = makeUnique<D3D12Core::DescriptorHeap>(Internal::numSamplerDescriptors,
		                                                    0,
		                                                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		                                                    D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

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
}

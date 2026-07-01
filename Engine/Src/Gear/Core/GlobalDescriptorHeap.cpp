#include<Gear/Core/GlobalDescriptorHeap.h>

#include<Gear/Core/Internal/GlobalDescriptorHeapInternal.h>

namespace Gear::Core::GlobalDescriptorHeap
{
	namespace Internal
	{
		struct GlobalDescriptorHeapImpl
		{
			GlobalDescriptorHeapImpl();

			UniquePtr<D3D12Core::DescriptorHeap> resourceHeap;

			UniquePtr<D3D12Core::DescriptorHeap> samplerHeap;
		};

		GlobalDescriptorHeapImpl::GlobalDescriptorHeapImpl()
		{
			resourceHeap = makeUnique<D3D12Core::DescriptorHeap>(numResourceHeapDescriptors, numResourceHeapDescriptors - numStaticCBVSRVUAVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

			samplerHeap = makeUnique<D3D12Core::DescriptorHeap>(1024, 0, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

			LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(GlobalDescriptorHeap));
		}

		UniquePtr<GlobalDescriptorHeapImpl> impl;

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
		return Internal::impl->resourceHeap.get();
	}

	D3D12Core::DescriptorHeap* getSamplerHeap()
	{
		return Internal::impl->samplerHeap.get();
	}
}


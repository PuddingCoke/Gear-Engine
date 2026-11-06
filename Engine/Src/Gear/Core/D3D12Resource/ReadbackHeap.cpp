#include <Gear/Core/D3D12Resource/ReadbackHeap.h>

namespace Gear::Core::D3D12Resource
{
	ReadbackHeap::ReadbackHeap(const uint64_t size, const D3D12_HEAP_FLAGS flags) :
		D3D12ResourceBase(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
	                      flags,
	                      CD3DX12_RESOURCE_DESC::Buffer(size),
	                      false,
	                      D3D12_RESOURCE_STATE_COPY_DEST,
	                      nullptr),
		size(size)
	{
	}

	ReadbackHeap::ReadbackHeap(ReadbackHeap& heap) : D3D12ResourceBase(heap), size(heap.size)
	{
	}

	ReadbackHeap::~ReadbackHeap()
	{
	}

	const void* ReadbackHeap::map(const uint64_t begin, const uint64_t end) const
	{
		void* dataPtr = nullptr;

		const CD3DX12_RANGE readRange = CD3DX12_RANGE(begin, end);

		getResource()->Map(0, &readRange, &dataPtr);

		return dataPtr;
	}

	const void* ReadbackHeap::map() const
	{
		return map(0ull, size);
	}

	void ReadbackHeap::unmap() const
	{
		getResource()->Unmap(0, nullptr);
	}

	uint64_t ReadbackHeap::getSize() const
	{
		return size;
	}
}

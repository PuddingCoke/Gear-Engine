#include<Gear/Core/Resource/D3D12Resource/ReadbackHeap.h>

Gear::Core::Resource::D3D12Resource::ReadbackHeap::ReadbackHeap(const uint64_t size, const D3D12_HEAP_FLAGS flags) :
	D3D12ResourceBase(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), flags, CD3DX12_RESOURCE_DESC::Buffer(size), false, D3D12_RESOURCE_STATE_COPY_DEST, nullptr)
{
}

Gear::Core::Resource::D3D12Resource::ReadbackHeap::ReadbackHeap(ReadbackHeap& heap) :
	D3D12ResourceBase(heap)
{
}

void* Gear::Core::Resource::D3D12Resource::ReadbackHeap::map(const D3D12_RANGE readRange) const
{
	void* dataPtr = nullptr;

	getResource()->Map(0, &readRange, &dataPtr);

	return dataPtr;
}

void Gear::Core::Resource::D3D12Resource::ReadbackHeap::unmap() const
{
	getResource()->Unmap(0, nullptr);
}

void Gear::Core::Resource::D3D12Resource::ReadbackHeap::updateGlobalStates()
{
}

void Gear::Core::Resource::D3D12Resource::ReadbackHeap::resetInternalStates()
{
}

void Gear::Core::Resource::D3D12Resource::ReadbackHeap::resetTransitionStates()
{
}

Gear::Core::Resource::D3D12Resource::ReadbackHeap::~ReadbackHeap()
{
}

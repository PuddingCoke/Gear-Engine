#include<Gear/Core/D3D12Core/DescriptorHeap.h>

Gear::Core::D3D12Core::DescriptorHeap::DescriptorHeap(const uint32_t numDescriptors, const uint32_t numDynamicDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE type, const D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
	numDescriptors(numDescriptors), numDynamicDescriptors(numDynamicDescriptors), type(type),
	incrementSize(GraphicsDevice::get()->GetDescriptorHandleIncrementSize(type)), staticIndex(0ull), dynamicIndex(0ull)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NodeMask = 0;
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = flags;

	GraphicsDevice::get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));

	cpuHandleStart = descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	if (flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		gpuHandleStart = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	}
	else
	{
		gpuHandleStart = CD3DX12_GPU_DESCRIPTOR_HANDLE();
	}
}

ID3D12DescriptorHeap* Gear::Core::D3D12Core::DescriptorHeap::get() const
{
	return descriptorHeap.Get();
}

Gear::Core::D3D12Core::DescriptorHandle Gear::Core::D3D12Core::DescriptorHeap::allocStaticDescriptor(const uint32_t num)
{
	const uint64_t retIndex = staticIndex.fetch_add(num, std::memory_order_relaxed);

	//这里要加个错误检测
#ifdef _DEBUG
	if (retIndex + num > numDescriptors - numDynamicDescriptors)
	{
		LOGERROR(L"staticIndex out of boundary");
	}
#endif // _DEBUG

	return DescriptorHandle(cpuHandleStart, gpuHandleStart, static_cast<uint32_t>(retIndex), incrementSize, 0u, numDescriptors, this);
}

Gear::Core::D3D12Core::DescriptorHandle Gear::Core::D3D12Core::DescriptorHeap::allocDynamicDescriptor(const uint32_t num)
{
	const uint64_t retIndex = dynamicIndex.fetch_add(num, std::memory_order_relaxed) % numDynamicDescriptors;

	return DescriptorHandle(cpuHandleStart, gpuHandleStart, static_cast<uint32_t>(retIndex), incrementSize, numDescriptors - numDynamicDescriptors, numDynamicDescriptors, this);
}

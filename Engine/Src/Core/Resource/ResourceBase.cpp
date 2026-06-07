#include<Gear/Core/Resource/ResourceBase.h>

namespace Gear::Core::Resource
{
	ResourceBase::ResourceBase(const bool persistent) :
		persistent(persistent), numCBVSRVUAVDescriptors(0), copySrcDescriptorHandle()
	{
	}

	ResourceBase::~ResourceBase()
	{
	}

	void ResourceBase::copyDescriptors()
	{
	}

	bool ResourceBase::getPersistent() const
	{
		return persistent;
	}

	uint32_t ResourceBase::getNumCBVSRVUAVDescriptors() const
	{
		return numCBVSRVUAVDescriptors;
	}

	void ResourceBase::setNumCBVSRVUAVDescriptors(const uint32_t numDescriptors)
	{
		numCBVSRVUAVDescriptors = numDescriptors;
	}

	D3D12Core::DescriptorHandle ResourceBase::allocCBVSRVUAVDescriptors()
	{
#ifdef _DEBUG
		if (getNumCBVSRVUAVDescriptors() == 0u)
		{
			LOGERROR(L"调用这个方法前应该先设置", TOWSTRING(numCBVSRVUAVDescriptors));
		}
#endif // _DEBUG

		if (persistent)
		{
			copySrcDescriptorHandle = GlobalDescriptorHeap::getResourceHeap()->allocStaticDescriptor(getNumCBVSRVUAVDescriptors());
		}
		else
		{
			copySrcDescriptorHandle = LocalDescriptorHeap::getStagingResourceHeap()->allocDynamicDescriptor(getNumCBVSRVUAVDescriptors());
		}

		return copySrcDescriptorHandle;
	}

	D3D12Core::DescriptorHandle ResourceBase::copyToResourceHeap() const
	{
#ifdef _DEBUG
		if (persistent)
		{
			LOGERROR(L"对于持久性资源来说，调用", TOWSTRING(copyToResourceHeap), L"是非法的！");
		}
#endif // _DEBUG

		D3D12Core::DescriptorHandle copyDestDescriptorHandle = GlobalDescriptorHeap::getResourceHeap()->allocDynamicDescriptor(getNumCBVSRVUAVDescriptors());

		D3D12Core::DescriptorHandle::copyDescriptors(getNumCBVSRVUAVDescriptors(), copyDestDescriptorHandle, copySrcDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		return copyDestDescriptorHandle;
	}
}
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

	Gear::Core::D3D12Core::DescriptorHandle ResourceBase::allocCBVSRVUAVDescriptors()
	{
#ifdef _DEBUG
		if (getNumCBVSRVUAVDescriptors() == 0u)
		{
			LOGERROR(L"should set numCBVSRVUAVDescriptors before using this method");
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

	Gear::Core::D3D12Core::DescriptorHandle ResourceBase::copyToResourceHeap() const
	{
#ifdef _DEBUG
		if (persistent)
		{
			LOGERROR(L"call getTransientDescriptorHandle for persistent resources is not allowed");
		}
#endif // _DEBUG

		D3D12Core::DescriptorHandle copyDestDescriptorHandle = GlobalDescriptorHeap::getResourceHeap()->allocDynamicDescriptor(getNumCBVSRVUAVDescriptors());

		D3D12Core::DescriptorHandle::copyDescriptors(getNumCBVSRVUAVDescriptors(), copyDestDescriptorHandle, copySrcDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		return copyDestDescriptorHandle;
	}
}
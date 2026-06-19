#include<Gear/Resource/ResourceBase.h>

namespace Gear::Resource
{
	ResourceBase::ResourceBase(const bool persistent) :
		persistent(persistent), numCBVSRVUAVDescriptors(0), copySrcDescriptorHandle(), lastUpdateDynamicIndex(UINT64_MAX)
	{
	}

	//拷贝时persistent无论如何都要被设置为false，因为每个渲染线程都有着独享的LocalDescriptorHeap
	//这意味着只能是创建那个资源的渲染线程调用资源的copyDescriptors方法
	ResourceBase::ResourceBase(const ResourceBase& resource) :
		persistent(false),
		numCBVSRVUAVDescriptors(resource.numCBVSRVUAVDescriptors),
		copySrcDescriptorHandle()
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
			LOGERROR(L"调用", TOWSTRING(allocCBVSRVUAVDescriptors), L"前应该先调用", TOWSTRING(setNumCBVSRVUAVDescriptors));
		}
#endif // _DEBUG

		if (getPersistent())
		{
			copySrcDescriptorHandle = GlobalDescriptorHeap::getResourceHeap()->allocStaticDescriptor(getNumCBVSRVUAVDescriptors());
		}
		else
		{
			copySrcDescriptorHandle = LocalDescriptorHeap::getStagingResourceHeap()->allocDynamicDescriptor(getNumCBVSRVUAVDescriptors());
		}

		return copySrcDescriptorHandle;
	}

	bool ResourceBase::copyToResourceHeap(D3D12Core::DescriptorHandle& descriptorHandle)
	{
#ifdef _DEBUG
		if (getPersistent())
		{
			LOGERROR(L"对于持久性资源来说，调用", TOWSTRING(copyToResourceHeap), L"是非法的！");
		}
#endif // _DEBUG

		if (lastUpdateDynamicIndex != UINT64_MAX)
		{
			//>=lastUpdateDynamicIndex
			const uint64_t currentDynamicIndex = GlobalDescriptorHeap::getResourceHeap()->getDynamicIndex();

			const uint64_t numDynamicDescriptors = GlobalDescriptorHeap::getResourceHeap()->getNumDynamicDescriptors();

			//获取lastUpdateDynamicIndex下一个循环的对应位置
			const uint64_t nextLoopDynamicIndex = lastUpdateDynamicIndex + numDynamicDescriptors;

			//安全余量
			const uint64_t safeMargin = numDynamicDescriptors * safeMarginNumerator / safeMarginDenominator;

			//如果大于安全余量就不更新
			if (nextLoopDynamicIndex > currentDynamicIndex && nextLoopDynamicIndex - currentDynamicIndex > safeMargin)
			{
				return false;
			}
		}

		D3D12Core::DescriptorHandle copyDestDescriptorHandle = GlobalDescriptorHeap::getResourceHeap()->allocDynamicDescriptor(getNumCBVSRVUAVDescriptors(), lastUpdateDynamicIndex);

		D3D12Core::DescriptorHandle::copyDescriptors(getNumCBVSRVUAVDescriptors(), copyDestDescriptorHandle, copySrcDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		descriptorHandle = copyDestDescriptorHandle;

		return true;
	}
}
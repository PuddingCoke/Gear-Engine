#include<Gear/Resource/ResourceBase.h>

#include<Gear/Core/RenderThreadLocal.h>

namespace Gear::Resource
{
	ResourceBase::ResourceBase(const bool persistent) :
		persistent(persistent), numCBVSRVUAVDescriptors(0), copySrcDescriptorHandle(), lastUpdateDynamicIndex(UINT64_MAX), currentFrameCopied(false)
	{
	}

	//拷贝时persistent无论如何都要被设置为false，因为每个渲染线程都有着独享的LocalDescriptorHeap
	//这意味着只能是创建那个资源的渲染线程调用资源的copyDescriptors方法
	ResourceBase::ResourceBase(const ResourceBase& resource) :
		persistent(false),
		numCBVSRVUAVDescriptors(resource.numCBVSRVUAVDescriptors),
		copySrcDescriptorHandle(),
		lastUpdateDynamicIndex(UINT64_MAX),
		currentFrameCopied(false)
	{
	}

	ResourceBase::~ResourceBase()
	{
	}

	bool ResourceBase::copyDescriptors()
	{
		return false;
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

	void ResourceBase::resetCopyState()
	{
		currentFrameCopied = false;
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

		//这一帧如果拷贝了就不再进行多余的拷贝
		if (currentFrameCopied)
		{
			return false;
		}

		//currentFrameCopied应该放置在这里，得保证每帧使用数值相同的资源索引
		currentFrameCopied = true;

		RenderThreadLocal::pushToCopiedResources(this);

		//如果没有拷贝过，但是检测到当前的位置是安全的那么也不更新，同时视为这一帧拷贝完毕
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
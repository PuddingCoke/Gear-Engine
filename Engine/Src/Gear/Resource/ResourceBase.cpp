#include <Gear/Resource/ResourceBase.h>

#include <Gear/Core/RenderThreadLocal.h>

#include <Gear/Core/Graphics.h>

namespace Gear::Resource
{
	ResourceBase::ResourceBase() :
		inVolatileResourceList(false),
		preparedFrameEpoch(makeShared<uint64_t>(UINT64_MAX)),
		prepareMutex(makeShared<std::mutex>())
	{
	}

	ResourceBase::ResourceBase(const ResourceBase& resource) :
		inVolatileResourceList(false),
		preparedFrameEpoch(resource.preparedFrameEpoch),
		prepareMutex(resource.prepareMutex),
		resourceDescriptorHandle(resource.resourceDescriptorHandle),
		nonShaderVisibleResourceDescriptorHandle(resource.nonShaderVisibleResourceDescriptorHandle),
		renderTargetDescriptorHandle(resource.renderTargetDescriptorHandle),
		depthStencilDescriptorHandle(resource.depthStencilDescriptorHandle)
	{
		//使用的描述符是全局堆上的，因此使用共享模式
	}

	ResourceBase::~ResourceBase()
	{
	}

	bool ResourceBase::hasVolatileDescriptor() const
	{
		return false;
	}

	void ResourceBase::prepareDescriptors()
	{
		if (!hasVolatileDescriptor())
		{
			THROWLOG(LOGERROR() << "对于无易变描述符的资源来说，不应该调用" << TOSTRING(prepareDescriptors));
		}

		std::lock_guard lock(*prepareMutex);

		if (*preparedFrameEpoch == Graphics::getFrameEpoch())
		{
			return;
		}

		prepareDescriptorsImpl();

		*preparedFrameEpoch = Graphics::getFrameEpoch();
	}

	bool ResourceBase::getInVolatileResourceList() const
	{
		return inVolatileResourceList;
	}

	void ResourceBase::pushToVolatileResourceList(std::vector<ResourceBase*>& volatileResourceList)
	{
		if (!inVolatileResourceList)
		{
			volatileResourceList.push_back(this);

			inVolatileResourceList = true;
		}
	}

	void ResourceBase::popFromVolatileResourceList()
	{
		inVolatileResourceList = false;
	}

	D3D12Core::DescriptorHandle* ResourceBase::createResourceDescriptorHandle(const uint32_t numDescriptors)
	{
		if (numDescriptors == 0)
		{
			return nullptr;
		}

		resourceDescriptorHandle =
			GlobalDescriptorHeap::getResourceHeap()->allocUnorderedDescriptor(numDescriptors);

		return getResourceDescriptorHandle();
	}

	D3D12Core::DescriptorHandle* ResourceBase::createNonShaderVisibleResourceDescriptorHandle(
		const uint32_t numDescriptors)
	{
		if (numDescriptors == 0)
		{
			return nullptr;
		}

		nonShaderVisibleResourceDescriptorHandle =
			GlobalDescriptorHeap::getNonShaderVisibleResourceHeap()->allocUnorderedDescriptor(numDescriptors);

		return getNonShaderVisibleResourceDescriptorHandle();
	}

	D3D12Core::DescriptorHandle* ResourceBase::createRenderTargetDescriptorHandle(
		const uint32_t numDescriptors)
	{
		if (numDescriptors == 0)
		{
			return nullptr;
		}

		renderTargetDescriptorHandle =
			GlobalDescriptorHeap::getRenderTargetHeap()->allocUnorderedDescriptor(numDescriptors);

		return getRenderTargetDescriptorHandle();
	}

	D3D12Core::DescriptorHandle* ResourceBase::createDepthStencilDescriptorHandle(
		const uint32_t numDescriptors)
	{
		if (numDescriptors == 0)
		{
			return nullptr;
		}

		depthStencilDescriptorHandle =
			GlobalDescriptorHeap::getDepthStencilHeap()->allocUnorderedDescriptor(numDescriptors);

		return getDepthStencilDescriptorHandle();
	}

	D3D12Core::DescriptorHandle* ResourceBase::getResourceDescriptorHandle() const
	{
		return resourceDescriptorHandle.get();
	}

	D3D12Core::DescriptorHandle* ResourceBase::getNonShaderVisibleResourceDescriptorHandle() const
	{
		return nonShaderVisibleResourceDescriptorHandle.get();
	}

	D3D12Core::DescriptorHandle* ResourceBase::getRenderTargetDescriptorHandle() const
	{
		return renderTargetDescriptorHandle.get();
	}

	D3D12Core::DescriptorHandle* ResourceBase::getDepthStencilDescriptorHandle() const
	{
		return depthStencilDescriptorHandle.get();
	}

	void ResourceBase::prepareDescriptorsImpl()
	{
	}

}

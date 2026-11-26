#include <Gear/Resource/ResourceBase.h>

#include <Gear/Core/RenderThreadLocal.h>

#include <Gear/Core/Graphics.h>

namespace Gear::Resource
{

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

}

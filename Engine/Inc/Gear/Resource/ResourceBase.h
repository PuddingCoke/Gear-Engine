#pragma once

#ifndef _GEAR_RESOURCE_RESOURCEBASE_H_
#define _GEAR_RESOURCE_RESOURCEBASE_H_

#include <Gear/Core/GlobalDescriptorHeap.h>

#include <Gear/Utils/Math.h>

#include "PipelineResourceDesc.h"

namespace Gear::Resource
{
	using namespace Gear::Core;

	class ResourceBase
	{
	public:

		ResourceBase() = default;

		ResourceBase(const ResourceBase&) = default;

		virtual ~ResourceBase() = default;

	protected:

		D3D12Core::DescriptorHandle* createResourceDescriptorHandle(const uint32_t numDescriptors);

		D3D12Core::DescriptorHandle* createNonShaderVisibleResourceDescriptorHandle(
			const uint32_t numDescriptors);

		D3D12Core::DescriptorHandle* createRenderTargetDescriptorHandle(const uint32_t numDescriptors);

		D3D12Core::DescriptorHandle* createDepthStencilDescriptorHandle(const uint32_t numDescriptors);

		D3D12Core::DescriptorHandle* getResourceDescriptorHandle() const;

		D3D12Core::DescriptorHandle* getNonShaderVisibleResourceDescriptorHandle() const;

		D3D12Core::DescriptorHandle* getRenderTargetDescriptorHandle() const;

		D3D12Core::DescriptorHandle* getDepthStencilDescriptorHandle() const;

	private:

		//使用的描述符是全局堆上的，因此使用共享模式

		SharedPtr<D3D12Core::DescriptorHandle> resourceDescriptorHandle;

		SharedPtr<D3D12Core::DescriptorHandle> nonShaderVisibleResourceDescriptorHandle;

		SharedPtr<D3D12Core::DescriptorHandle> renderTargetDescriptorHandle;

		SharedPtr<D3D12Core::DescriptorHandle> depthStencilDescriptorHandle;
	};
}

#endif // !_GEAR_RESOURCE_RESOURCEBASE_H_

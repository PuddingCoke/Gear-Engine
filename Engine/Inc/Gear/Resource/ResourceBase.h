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

		//初始化
		ResourceBase();

		//深拷贝
		ResourceBase(const ResourceBase&);

		virtual ~ResourceBase();

		virtual bool hasVolatileDescriptor() const;

		//准备描述符（引擎内部使用）
		void prepareDescriptors();

		bool getInVolatileResourceList() const;

		void pushToVolatileResourceList(std::vector<ResourceBase*>& volatileResourceList);

		void popFromVolatileResourceList();

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

		virtual void prepareDescriptorsImpl();

	private:

		bool inVolatileResourceList;

		//由于指令是并行录制的，所以各个渲染线程需要竞争调用 prepareDescriptors
		//因此需要记录上一次调用 prepareDescriptors 的帧纪元和互斥锁

		SharedPtr<uint64_t> preparedFrameEpoch;

		SharedPtr<std::mutex> prepareMutex;

		SharedPtr<D3D12Core::DescriptorHandle> resourceDescriptorHandle;

		SharedPtr<D3D12Core::DescriptorHandle> nonShaderVisibleResourceDescriptorHandle;

		SharedPtr<D3D12Core::DescriptorHandle> renderTargetDescriptorHandle;

		SharedPtr<D3D12Core::DescriptorHandle> depthStencilDescriptorHandle;
	};
}

#endif // !_GEAR_RESOURCE_RESOURCEBASE_H_

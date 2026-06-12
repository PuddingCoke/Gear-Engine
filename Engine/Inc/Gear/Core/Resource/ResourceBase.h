#pragma once

#ifndef _GEAR_CORE_RESOURCE_RESOURCEBASE_H_
#define _GEAR_CORE_RESOURCE_RESOURCEBASE_H_

#include<Gear/Core/GlobalDescriptorHeap.h>

#include<Gear/Core/LocalDescriptorHeap.h>

#include"D3D12Resource/PipelineResourceDesc.h"

namespace Gear::Core::Resource
{
	class ResourceBase
	{
	public:

		ResourceBase() = delete;

		//初始化
		ResourceBase(const bool persistent);

		//深拷贝
		ResourceBase(const ResourceBase&);

		virtual ~ResourceBase();

		virtual void copyDescriptors() = 0;

		bool getPersistent() const;

		uint32_t getNumCBVSRVUAVDescriptors() const;

		void setNumCBVSRVUAVDescriptors(const uint32_t numDescriptors);

	protected:

		//从描述器堆申请可用的描述符并返回DescriptorHandle
		//在调用前要设置numCBVSRVUAVDescriptors！
		D3D12Core::DescriptorHandle allocCBVSRVUAVDescriptors();

		//把非持久性资源的描述符拷贝到资源描述符堆上，并返回DescriptorHandle
		//仅用于非持久性资源！
		D3D12Core::DescriptorHandle copyToResourceHeap() const;

		const bool persistent;

	private:

		//资源需要的CBV、SRV、UAV描述符的数量
		uint32_t numCBVSRVUAVDescriptors;

		//对于非持久性资源的拷贝开始位置
		D3D12Core::DescriptorHandle copySrcDescriptorHandle;

	};
}

#endif // !_GEAR_CORE_RESOURCE_RESOURCEBASE_H_
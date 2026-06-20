#pragma once

#ifndef _GEAR_RESOURCE_RESOURCEBASE_H_
#define _GEAR_RESOURCE_RESOURCEBASE_H_

#include<Gear/Core/GlobalDescriptorHeap.h>

#include<Gear/Core/LocalDescriptorHeap.h>

#include<Gear/Utils/Math.h>

#include"PipelineResourceDesc.h"

namespace Gear::Resource
{
	using namespace Gear::Core;

	class ResourceBase
	{
	public:

		ResourceBase() = delete;

		//初始化
		ResourceBase(const bool persistent);

		//深拷贝
		ResourceBase(const ResourceBase&);

		virtual ~ResourceBase();

		virtual bool copyDescriptors();

		bool getPersistent() const;

		uint32_t getNumCBVSRVUAVDescriptors() const;

		void setNumCBVSRVUAVDescriptors(const uint32_t numDescriptors);

		void resetCopyState();

	protected:

		//从描述器堆申请可用的描述符并返回DescriptorHandle
		//在调用前要设置numCBVSRVUAVDescriptors！
		D3D12Core::DescriptorHandle allocCBVSRVUAVDescriptors();

		//把非持久性资源的描述符从线程独享的暂存资源描述符堆拷贝到全局资源描述符堆上
		//返回值代表是否需要更新
		//注意：仅用于非持久性资源！
		bool copyToResourceHeap(D3D12Core::DescriptorHandle& descriptorHandle);

		const bool persistent;

	private:

		static constexpr uint64_t safeMarginNumerator = 1;

		static constexpr uint64_t safeMarginDenominator = 2;

		bool currentFrameCopied;

		uint64_t lastUpdateDynamicIndex;

		//资源需要的CBV、SRV、UAV描述符的数量
		uint32_t numCBVSRVUAVDescriptors;

		//对于非持久性资源的拷贝开始位置
		D3D12Core::DescriptorHandle copySrcDescriptorHandle;

	};
}

#endif // !_GEAR_RESOURCE_RESOURCEBASE_H_
#pragma once

#ifndef _GEAR_RESOURCE_DYNAMICCBUFFER_H_
#define _GEAR_RESOURCE_DYNAMICCBUFFER_H_

#include "ImmutableCBuffer.h"

namespace Gear::Resource
{
	CREATESAFETYPE(DynamicCBuffer);

	//注意：每帧都必须更新一次
	class DynamicCBuffer : public ImmutableCBuffer
	{
	public:

		DynamicCBuffer(const uint32_t sizeParam);

		DynamicCBuffer(const DynamicCBuffer&);

		//更新CPU侧的缓冲区，用户只需调用此方法
		void update(const void* const data, const uint32_t updateSize);

		bool hasVolatileDescriptor() const override;

		//获取可用的GPU映射指针（引擎内部使用）
		void acquireMappedDataPtr();

		//更新GPU侧的映射指针指向的数据（引擎内部使用）
		void updateMappedData();

	private:

		void prepareDescriptorsImpl() override;

		//根据请求大小向上取整得到的区域索引，必须先于size初始化
		const uint32_t regionIndex;

		//区域大小（256 << regionIndex），也是影子缓冲和上传的长度
		const uint32_t size;

		//CPU侧的缓冲区，用于暂存数据
		SharedPtr<uint8_t[]> dataPtr;

		//GPU侧的映射指针
		SharedPtr<void*> mappedDataPtr;

		SharedPtr<uint64_t> updateDataPtrFrameEpoch;

		SharedPtr<uint64_t> acquireFrameEpoch;

		SharedPtr<uint64_t> updateMappedDataPtrFrameEpoch;
	};
}

#endif // !_GEAR_RESOURCE_DYNAMICCBUFFER_H_

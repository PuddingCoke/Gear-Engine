#pragma once

#ifndef _GEAR_CORE_RESOURCE_DYNAMICCBUFFER_H_
#define _GEAR_CORE_RESOURCE_DYNAMICCBUFFER_H_

#include"ImmutableCBuffer.h"

namespace Gear::Core::Resource
{
	CREATESAFETYPE(DynamicCBuffer);

	//硬性要求：每帧都必须更新一次
	class DynamicCBuffer :public ImmutableCBuffer
	{
	public:

		DynamicCBuffer(const uint32_t size);

		DynamicCBuffer(const DynamicCBuffer&);

		//获取可用的数据指针
		void acquireDataPtr();

		//更新数据指针的内容
		void updateData(const void* const data);

		//先获取再更新数据指针的内容
		void simpleUpdate(const void* const data);

	private:

		const uint32_t regionIndex;

		SharedPtr<void*> dataPtr;

		SharedPtr<uint64_t> acquireFrameIndex;

		SharedPtr<uint64_t> updateFrameIndex;

	};
}

#endif // !_GEAR_CORE_RESOURCE_DYNAMICCBUFFER_H_

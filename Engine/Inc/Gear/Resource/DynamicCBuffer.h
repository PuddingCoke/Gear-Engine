#pragma once

#ifndef _GEAR_RESOURCE_DYNAMICCBUFFER_H_
#define _GEAR_RESOURCE_DYNAMICCBUFFER_H_

#include <Gear/Core/DynamicCBufferManager.h>

#include "ImmutableCBuffer.h"

namespace Gear::Resource
{
	CREATESAFETYPE(DynamicCBuffer);

	//注意：每帧都必须更新一次
	class DynamicCBuffer : public ImmutableCBuffer
	{
	public:

		DynamicCBuffer(const uint32_t size);

		DynamicCBuffer(const DynamicCBuffer&);

		~DynamicCBuffer();

		//更新映射缓冲区，updateSize不能大于缓冲区实际大小
		void update(const void* const data, const uint32_t updateSize);

		//获取缓冲区实际大小
		uint32_t getSize() const;

	private:

		DynamicCBufferManager::AvailableLocationPtr location;

		SharedPtr<std::mutex> updateMutex;

		SharedPtr<uint64_t> updateFrameEpoch;
	};
}

#endif // !_GEAR_RESOURCE_DYNAMICCBUFFER_H_

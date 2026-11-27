#pragma once

#ifndef _GEAR_RESOURCE_DYNAMICCBUFFER_H_
#define _GEAR_RESOURCE_DYNAMICCBUFFER_H_

#include <Gear/Core/DynamicCBufferManager.h>

#include "ImmutableCBuffer.h"

namespace Gear::Resource
{
	CREATESAFETYPE(DynamicCBuffer);

	//注意：不必每帧都更新，因为内部实现使用影子缓冲区
	class DynamicCBuffer : public ImmutableCBuffer
	{
	public:

		DynamicCBuffer(const uint32_t size);

		DynamicCBuffer(const DynamicCBuffer&);

		~DynamicCBuffer();

		//更新影子缓冲区，updateSize不能大于影子缓冲区实际大小
		void update(const void* const data, const uint32_t updateSize);

		//获取影子缓冲区指针，支持随机读写，但是不能超出影子缓冲区实际大小
		void* getDataPtr() const;

		//获取影子缓冲区实际大小
		uint32_t getSize() const;

	private:

		DynamicCBufferManager::AvailableLocationPtr location;
	};
}

#endif // !_GEAR_RESOURCE_DYNAMICCBUFFER_H_

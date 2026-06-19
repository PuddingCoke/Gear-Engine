#pragma once

#ifndef _GEAR_CORE_IMMUTABLEINDEXCBUFFER_H_
#define _GEAR_CORE_IMMUTABLEINDEXCBUFFER_H_

#include"IndexCBufferBase.h"

#include<Gear/Resource/ImmutableCBuffer.h>

namespace Gear::Core
{
	CREATESAFETYPE(ImmutableIndexCBuffer);

	class ImmutableIndexCBuffer :public IndexCBufferBase, public Resource::ImmutableCBuffer
	{
		template<size_t N>
		ImmutableIndexCBuffer(const ResourceIndexPair(&pairs)[N], D3D12Resource::BufferPtr bufferPtr, const uint32_t size, const bool persistent);
	};

	template<size_t N>
	inline ImmutableIndexCBuffer::ImmutableIndexCBuffer(const ResourceIndexPair(&pairs)[N], D3D12Resource::BufferPtr bufferPtr, const uint32_t size, const bool persistent) :
		IndexCBufferBase(pairs),
		ImmutableCBuffer(bufferPtr, size, persistent)
	{
	}
}

#endif // !_GEAR_CORE_IMMUTABLEINDEXCBUFFER_H_

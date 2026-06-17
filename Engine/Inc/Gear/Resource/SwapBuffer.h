#pragma once

#ifndef _GEAR_CORE_RESOURCE_SWAPBUFFER_H_
#define _GEAR_CORE_RESOURCE_SWAPBUFFER_H_

#include"BufferView.h"

#include"SwappableBase.h"

namespace Gear::Resource
{
	using SwapBuffer = SwappableBase<BufferView>;

	using SwapBufferPtr = UniquePtr<SwapBuffer>;
}

#endif // !_GEAR_CORE_RESOURCE_SWAPBUFFER_H_

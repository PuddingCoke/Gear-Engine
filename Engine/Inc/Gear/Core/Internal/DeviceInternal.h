#pragma once

#ifndef _GEAR_CORE_DEVICE_INTERNAL_H_
#define _GEAR_CORE_DEVICE_INTERNAL_H_

#include<directx/d3dx12.h>

namespace Gear::Core::Device::Internal
{
	void initialize(IUnknown* const adapter);

	void release();

	struct InitializeToken { InitializeToken(IUnknown* const adapter) { initialize(adapter); } ~InitializeToken() { release(); } };
}

#endif // !_GEAR_CORE_DEVICE_INTERNAL_H_

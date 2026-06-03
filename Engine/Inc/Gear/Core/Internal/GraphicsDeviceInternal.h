#pragma once

#ifndef _GEAR_CORE_GRAPHICSDEVICE_INTERNAL_H_
#define _GEAR_CORE_GRAPHICSDEVICE_INTERNAL_H_

#include<D3D12Headers/d3dx12.h>

namespace Gear::Core::GraphicsDevice::Internal
{
	void initialize(IUnknown* const adapter);

	void release();

	struct InitializeToken { InitializeToken(IUnknown* const adapter) { initialize(adapter); } ~InitializeToken() { release(); } };

	void checkFeatureSupport();
}

#endif // !_GEAR_CORE_GRAPHICSDEVICE_INTERNAL_H_

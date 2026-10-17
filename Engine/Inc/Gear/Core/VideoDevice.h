#pragma once

#ifndef _GEAR_CORE_VIDEODEVICE_H_
#define _GEAR_CORE_VIDEODEVICE_H_

#include<directx/d3d12video.h>

namespace Gear::Core::VideoDevice
{
	ID3D12VideoDevice3* get();
}

#endif // !_GEAR_CORE_VIDEODEVICE_H_

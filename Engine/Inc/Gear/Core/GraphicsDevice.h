#pragma once

#ifndef _GEAR_CORE_GRAPHICSDEVICE_H_
#define _GEAR_CORE_GRAPHICSDEVICE_H_

static_assert(sizeof(int) == 4, "int类型的大小必须是4");

static_assert(sizeof(float) == 4, "float类型的大小必须是4");

static_assert(sizeof(double) == 8, "double类型的大小必须是8");

#include<Gear/Utils/Log.h>

#include<Gear/Utils/Memory.h>

#include<directx/d3dx12.h>

#include<comdef.h>

#define CHECKERROR(x) do\
{\
const HRESULT _hr_ = x;\
if(FAILED(_hr_))\
{\
_com_error _err_(_hr_);\
THROWLOG(LOGERROR() << "失败值" << LogIntegerMode::HEX << static_cast<uint32_t>(_hr_) << "失败原因" << _err_.ErrorMessage());\
}\
} while(0)\

#undef DOMAIN

namespace Gear::Core::GraphicsDevice
{
	ID3D12Device9* get();
}

#endif // !_GEAR_CORE_GRAPHICSDEVICE_H_

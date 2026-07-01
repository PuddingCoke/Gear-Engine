#pragma once

#ifndef _GEAR_CORE_GRAPHICSDEVICE_H_
#define _GEAR_CORE_GRAPHICSDEVICE_H_

static_assert(sizeof(int) == 4, "size of int must be 4");

static_assert(sizeof(float) == 4, "size of float must be 4");

static_assert(sizeof(double) == 8, "size of double must be 8");

#include<Gear/Utils/Logger.h>

#include<Gear/Utils/Memory.h>

#include<D3D12Headers/d3dx12.h>

#include<comdef.h>

#define CHECKERROR(x) do\
{\
const HRESULT _hr_ = x;\
if(FAILED(_hr_))\
{\
_com_error _err_(_hr_);\
LOGERROR(L"失败值",IntegerMode::HEX,static_cast<uint32_t>(_hr_),L"失败原因",_err_.ErrorMessage());\
}\
} while(0)\

#undef DOMAIN

namespace Gear::Core::GraphicsDevice
{
	ID3D12Device9* get();
}

#endif // !_GEAR_CORE_GRAPHICSDEVICE_H_
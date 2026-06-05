#pragma once

#ifndef _GEAR_CORE_GRAPHICSDEVICE_H_
#define _GEAR_CORE_GRAPHICSDEVICE_H_

static_assert(sizeof(int) == 4, "size of int must be 4");

static_assert(sizeof(float) == 4, "size of float must be 4");

static_assert(sizeof(double) == 8, "size of double must be 8");

#include<Gear/Utils/Logger.h>

#include<Gear/Utils/GearMemory.h>

#include<Gear/Core/TOPOLOGY.h>

#include<D3D12Headers/d3dx12.h>

#include<dxgi1_6.h>

#include<wrl/client.h>

#include<comdef.h>

using Microsoft::WRL::ComPtr;

#define CHECKERROR(x) \
{\
const HRESULT hr = x;\
if(FAILED(hr))\
{\
_com_error err(hr);\
LOGERROR(L"Failed with",IntegerMode::HEX,static_cast<uint32_t>(hr),L"Failed reason",err.ErrorMessage());\
}\
}\

#define TOWSTRING(x) L#x

#undef DOMAIN

namespace Gear::Core::GraphicsDevice
{
	ID3D12Device9* get();
}

#endif // !_GEAR_CORE_GRAPHICSDEVICE_H_
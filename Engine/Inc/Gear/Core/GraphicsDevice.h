#pragma once

#ifndef _GEAR_CORE_GRAPHICSDEVICE_H_
#define _GEAR_CORE_GRAPHICSDEVICE_H_

static_assert(sizeof(int) == 4, "size of int must be 4");

static_assert(sizeof(float) == 4, "size of float must be 4");

static_assert(sizeof(double) == 8, "size of double must be 8");

#include<Gear/Utils/Logger.h>

#include<Gear/Core/TOPOLOGY.h>

#include<D3D12Headers/d3dx12.h>

#include<dxgi1_6.h>

#include<wrl/client.h>

#include<comdef.h>

#include<memory>

using Microsoft::WRL::ComPtr;

template<typename T, typename Deleter = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, Deleter>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

#define CHECKERROR(x) \
{\
const HRESULT hr = x;\
if(FAILED(hr))\
{\
_com_error err(hr);\
LOGERROR(L"Failed with",IntegerMode::HEX,static_cast<uint32_t>(hr),L"Failed reason",err.ErrorMessage());\
}\
}\

#undef DOMAIN

namespace Gear
{
	namespace Core
	{
		namespace GraphicsDevice
		{
			ID3D12Device9* get();
		};
	}
}

#endif // !_GEAR_CORE_GRAPHICSDEVICE_H_
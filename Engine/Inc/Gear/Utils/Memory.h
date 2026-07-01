#pragma once

#ifndef _GEAR_UTILS_MEMORY_H_
#define _GEAR_UTILS_MEMORY_H_

#include<memory>

#include<wrl/client.h>

using Microsoft::WRL::ComPtr;

template<typename T, typename Deleter = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, Deleter>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

#define CREATESAFETYPE(x) class x; using x##Ptr = UniquePtr<x>

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

#undef min

#undef max

#endif // !_GEAR_UTILS_MEMORY_H_
#pragma once

#ifndef _GEAR_UTILS_H_
#define _GEAR_UTILS_H_

#include<memory>

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

#endif // !_GEAR_UTILS_H_
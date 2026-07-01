#pragma once

#ifndef _GEAR_RESOURCE_SWAPPABLEBASE_H_
#define _GEAR_RESOURCE_SWAPPABLEBASE_H_

#include<functional>

namespace Gear::Resource
{
	template<typename T>
	class SwappableBase
	{
	public:

		SwappableBase() = delete;

		SwappableBase(const std::function<UniquePtr<T>(void)>& readObjectFunc, const std::function<UniquePtr<T>(void)>& writeObjectFunc);

		virtual ~SwappableBase();

		T* read() const;

		T* write() const;

		void swap();

	private:

		UniquePtr<T> readObject;

		UniquePtr<T> writeObject;

	};

	template<typename T>
	inline SwappableBase<T>::SwappableBase(const std::function<UniquePtr<T>(void)>& readObjectFunc, const std::function<UniquePtr<T>(void)>& writeObjectFunc) :
		readObject(readObjectFunc()), writeObject(writeObjectFunc())
	{
	}

	template<typename T>
	inline SwappableBase<T>::~SwappableBase()
	{
	}

	template<typename T>
	inline T* SwappableBase<T>::read() const
	{
		return readObject.get();
	}

	template<typename T>
	inline T* SwappableBase<T>::write() const
	{
		return writeObject.get();
	}

	template<typename T>
	inline void SwappableBase<T>::swap()
	{
		readObject.swap(writeObject);
	}
}

#endif // !_GEAR_RESOURCE_SWAPPABLEBASE_H_

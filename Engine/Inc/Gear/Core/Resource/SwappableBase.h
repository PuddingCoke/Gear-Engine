#pragma once

#ifndef _GEAR_CORE_RESOURCE_SWAPPABLEBASE_H_
#define _GEAR_CORE_RESOURCE_SWAPPABLEBASE_H_

#include<functional>

namespace Gear
{
	namespace Core
	{
		namespace Resource
		{
			template<typename T>
			class SwappableBase
			{
			public:

				SwappableBase(const std::function<T* (void)>& readObjectFunc, const std::function<T* (void)>& writeObjectFunc);

				SwappableBase(const std::function<T* (void)>& objectFunc);

				~SwappableBase();

				T* read() const;

				T* write() const;

				void swap();

			private:

				T* readObject;

				T* writeObject;

			};

			template<typename T>
			inline SwappableBase<T>::SwappableBase(const std::function<T* (void)>& readObjectFunc, const std::function<T* (void)>& writeObjectFunc) :
				readObject(readObjectFunc()), writeObject(writeObjectFunc())
			{
			}

			template<typename T>
			inline SwappableBase<T>::SwappableBase(const std::function<T* (void)>& objectFunc) :
				SwappableBase<T>(objectFunc, objectFunc)
			{
			}

			template<typename T>
			inline SwappableBase<T>::~SwappableBase()
			{
				if (readObject)
					delete readObject;

				if (writeObject)
					delete writeObject;
			}

			template<typename T>
			inline T* SwappableBase<T>::read() const
			{
				return readObject;
			}

			template<typename T>
			inline T* SwappableBase<T>::write() const
			{
				return writeObject;
			}

			template<typename T>
			inline void SwappableBase<T>::swap()
			{
				T* const tempObject = readObject;

				readObject = writeObject;

				writeObject = tempObject;
			}
		}
	}
}

#endif // !_GEAR_CORE_RESOURCE_SWAPPABLEBASE_H_

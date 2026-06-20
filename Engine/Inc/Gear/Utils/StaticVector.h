#pragma once

#ifndef _GEAR_UTILS_STATICVECTOR_H_
#define _GEAR_UTILS_STATICVECTOR_H_

#include<array>

#include"Logger.h"

namespace Gear::Utils
{
	template<typename T, size_t N>
	class StaticVector
	{
	public:

		StaticVector();

		~StaticVector();

		size_t size() const;

		size_t byteSize() const;

		void push(const T& t);

		void clear();

		const T* data() const;

		const T* begin() const;

		const T* end() const;

		T* begin();

		T* end();

	private:

		std::array<T, N> arr;

		size_t idx;

	};

	template<typename T, size_t N>
	inline StaticVector<T, N>::StaticVector() :
		arr(), idx(0ull)
	{
	}

	template<typename T, size_t N>
	inline StaticVector<T, N>::~StaticVector()
	{
	}

	template<typename T, size_t N>
	inline size_t StaticVector<T, N>::size() const
	{
		return idx;
	}

	template<typename T, size_t N>
	inline size_t StaticVector<T, N>::byteSize() const
	{
		return idx * sizeof(T);
	}

	template<typename T, size_t N>
	inline void StaticVector<T, N>::push(const T& t)
	{
#ifdef _DEBUG
		if (idx >= N)
		{
			LOGERROR(L"超出", N, L"的存储上限");
		}
#endif // _DEBUG

		arr[idx++] = t;
	}

	template<typename T, size_t N>
	inline void StaticVector<T, N>::clear()
	{
		idx = 0ull;
	}

	template<typename T, size_t N>
	inline const T* StaticVector<T, N>::data() const
	{
		return arr.data();
	}

	template<typename T, size_t N>
	inline const T* StaticVector<T, N>::begin() const
	{
		return arr.data();
	}

	template<typename T, size_t N>
	inline const T* StaticVector<T, N>::end() const
	{
		return arr.data() + idx;
	}

	template<typename T, size_t N>
	inline T* StaticVector<T, N>::begin()
	{
		return arr.data();
	}

	template<typename T, size_t N>
	inline T* StaticVector<T, N>::end()
	{
		return arr.data() + idx;
	}
}

#endif // !_GEAR_UTILS_STATICVECTOR_H_
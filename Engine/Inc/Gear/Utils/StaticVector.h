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

		//每个元素的字节大小
		static size_t elementByteSize();

		//当前的元素个数
		size_t size() const;

		//容器的总元素个数
		static size_t totalSize();

		//当前的字节大小
		size_t byteSize() const;

		//容器的总字节大小
		static size_t totalByteSize();

		void push(const T& t);

		void clear();

		const T* data() const;

		const T* begin() const;

		const T* end() const;

		T* begin();

		T* end();

		T& front();

		T& back();

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
	inline size_t StaticVector<T, N>::elementByteSize()
	{
		return sizeof(T);
	}

	template<typename T, size_t N>
	inline size_t StaticVector<T, N>::size() const
	{
		return idx;
	}

	template<typename T, size_t N>
	inline size_t StaticVector<T, N>::totalSize()
	{
		return N;
	}

	template<typename T, size_t N>
	inline size_t StaticVector<T, N>::byteSize() const
	{
		return size() * elementByteSize();
	}

	template<typename T, size_t N>
	inline size_t StaticVector<T, N>::totalByteSize()
	{
		return totalSize() * elementByteSize();
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

	template<typename T, size_t N>
	inline T& StaticVector<T, N>::front()
	{
#ifdef _DEBUG
		if (idx == 0ull)
		{
			LOGERROR(L"无法获取起始元素");
		}
#endif // _DEBUG

		return arr[0];
	}

	template<typename T, size_t N>
	inline T& StaticVector<T, N>::back()
	{
#ifdef _DEBUG
		if (idx == 0ull)
		{
			LOGERROR(L"无法获取末尾元素");
		}
#endif // _DEBUG

		return arr[idx - 1ull];
	}
}

#endif // !_GEAR_UTILS_STATICVECTOR_H_
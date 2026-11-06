#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <Gear/Utils/String.h>

#include <Windows.h>

namespace Gear::Utils::String
{
	std::string wStringToString(const std::wstring& wstr)
	{
		if (wstr.empty())
		{
			return "";
		}

		const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);

		if (sizeNeeded == 0)
		{
			return "WideCharToMultiByte调用失败";
		}

		std::string result(sizeNeeded - 1, 0);

		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, &result[0], sizeNeeded, nullptr, nullptr);

		return result;
	}

	std::wstring stringToWString(const std::string& str)
	{
		if (str.empty())
		{
			return L"";
		}

		const int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, nullptr, 0);

		if (sizeNeeded == 0)
		{
			return L"MultiByteToWideChar调用失败";
		}

		std::wstring result(sizeNeeded - 1, 0);

		MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, &result[0], sizeNeeded);

		return result;
	}
}

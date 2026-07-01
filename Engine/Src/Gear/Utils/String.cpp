#include<Gear/Utils/String.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#include<Windows.h>

namespace Gear::Utils::String
{
	std::string wstringToString(const std::wstring& wstr)
	{
		if (wstr.empty())
		{
			return "";
		}

		const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);

		std::string result(sizeNeeded, 0);

		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), &result[0], sizeNeeded, nullptr, nullptr);

		return result;
	}

	std::wstring stringToWstring(const std::string& str)
	{
		if (str.empty())
		{
			return L"";
		}

		const int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);

		std::wstring result(sizeNeeded, 0);

		MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &result[0], sizeNeeded);

		return result;
	}
}
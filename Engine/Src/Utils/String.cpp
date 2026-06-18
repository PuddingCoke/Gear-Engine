#include<Gear/Utils/String.h>

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
}
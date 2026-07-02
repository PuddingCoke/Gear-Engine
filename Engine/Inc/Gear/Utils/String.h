#pragma once

#ifndef _GEAR_UTILS_STRING_H_
#define _GEAR_UTILS_STRING_H_

#include<string>

namespace Gear::Utils::String
{
	std::string wstringToString(const std::wstring& wstr);

	std::wstring stringToWString(const std::string& str);
}

#endif // !_GEAR_UTILS_STRING_H_

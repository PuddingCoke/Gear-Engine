#pragma once

#ifndef _GEAR_UTILS_FILE_INTERNAL_H_
#define _GEAR_UTILS_FILE_INTERNAL_H_

#include<string>

namespace Gear::Utils::File::Internal
{
	void initialize(const std::wstring& rootFolder);

	void release();

	struct InitializeToken { InitializeToken(const std::wstring& rootFolder) { initialize(rootFolder); } ~InitializeToken() { release(); } };
}

#endif // !_GEAR_UTILS_FILE_INTERNAL_H_

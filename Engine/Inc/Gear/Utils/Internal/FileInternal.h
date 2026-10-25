#pragma once

#ifndef _GEAR_UTILS_FILE_INTERNAL_H_
#define _GEAR_UTILS_FILE_INTERNAL_H_

namespace Gear::Utils::File::Internal
{
	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };
}

#endif // !_GEAR_UTILS_FILE_INTERNAL_H_

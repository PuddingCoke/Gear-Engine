#pragma once

#ifndef _GEAR_UTILS_WALLPAPERHELPER_H_
#define _GEAR_UTILS_WALLPAPERHELPER_H_

#include<cstdint>

#define NOMINMAX

#include<Windows.h>

namespace Gear::Utils::WallpaperHelper
{
	void getSystemResolution(uint32_t& width, uint32_t& height);

	HWND getWallpaperHWND();

	bool isDesktopObscured();

	void initialize();

	void release();

	struct DetectThreadToken { DetectThreadToken() { initialize(); } ~DetectThreadToken() { release(); } };

	//壁纸模式时，后台线程的检查间隔（毫秒）
	constexpr size_t obscureCheckInterval = 700;
}

#endif // !_GEAR_UTILS_WALLPAPERHELPER_H_

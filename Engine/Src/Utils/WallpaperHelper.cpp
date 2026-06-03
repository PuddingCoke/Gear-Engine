#include<Gear/Utils/WallpaperHelper.h>

namespace Gear::Utils::WallpaperHelper
{
	void getSystemResolution(uint32_t& width, uint32_t& height)
	{
		HMONITOR monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);

		MONITORINFOEX monitorInfo;
		monitorInfo.cbSize = sizeof(MONITORINFOEX);

		GetMonitorInfo(monitor, &monitorInfo);

		DEVMODE devMode;
		devMode.dmSize = sizeof(DEVMODE);
		devMode.dmDriverExtra = 0;

		EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);

		width = devMode.dmPelsWidth;

		height = devMode.dmPelsHeight;
	}

	HWND getWallpaperHWND()
	{
		const HWND progman = FindWindow(L"ProgMan", nullptr);

		SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

		HWND wallpaperHWND = nullptr;

		auto EnumWindowsProc = [](HWND hwnd, LPARAM lParam)->BOOL
			{
				HWND p = FindWindowEx(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
				HWND* ret = (HWND*)lParam;
				if (p)
				{
					*ret = FindWindowEx(nullptr, hwnd, L"WorkerW", nullptr);
				}
				return true;
			};

		EnumWindows(EnumWindowsProc, (LPARAM)&wallpaperHWND);

		return wallpaperHWND;
	}
}
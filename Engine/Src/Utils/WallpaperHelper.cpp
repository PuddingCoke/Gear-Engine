#include<Gear/Utils/WallpaperHelper.h>

#include<Gear/Utils/Logger.h>

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

	bool isDesktopObscured()
	{
		uint32_t w;

		uint32_t h;

		getSystemResolution(w, h);

		uint32_t covered = 0;

		uint32_t total = 0;

		const uint32_t taskBarHeight = h / 9;

		const uint32_t startY = h / 12;

		for (uint32_t y = startY; y < h - taskBarHeight; y += h / 9)
		{
			for (uint32_t x = startY; x < w; x += w / 16)
			{
				const HWND hWnd = WindowFromPoint({ static_cast<LONG>(x), static_cast<LONG>(y) });

				if (!hWnd || !IsWindowVisible(hWnd))
				{
					total++;

					continue;
				}

				wchar_t className[256];

				GetClassNameW(hWnd, className, 256);

				bool isDesktop = (_wcsicmp(className, L"Progman") == 0) || (_wcsicmp(className, L"WorkerW") == 0) || (hWnd == GetDesktopWindow());

				if (!isDesktop && _wcsicmp(className, L"SysListView32") == 0)
				{
					HWND parent = GetParent(hWnd);

					if (parent)
					{
						GetClassNameW(parent, className, 256);

						if (_wcsicmp(className, L"SHELLDLL_DefView") == 0)
						{
							HWND root = GetAncestor(hWnd, GA_ROOT);

							if (root)
							{
								GetClassNameW(root, className, 256);

								if ((_wcsicmp(className, L"Progman") == 0 || _wcsicmp(className, L"WorkerW") == 0))
								{
									isDesktop = true;
								}
							}
						}
					}
				}

				if (!isDesktop)
				{
					covered++;
				}

				total++;
			}
		}

		//LOGENGINE(static_cast<float>(covered) / static_cast<float>(total));

		return total > 0 && static_cast<float>(covered) / static_cast<float>(total) > 0.94f;
	}
}
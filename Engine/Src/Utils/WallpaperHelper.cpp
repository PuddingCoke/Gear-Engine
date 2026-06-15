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

	class DesktopDetectThread
	{
	public:

		DesktopDetectThread() :
			desktopObscured(false), running(true)
		{
			getSystemResolution(w, h);

			detectThread = std::thread(&DesktopDetectThread::detectLoop, this);
		}

		~DesktopDetectThread()
		{
			running.store(false);

			detectThread.join();
		}

		bool getDesktopObscured() const
		{
			return desktopObscured.load(std::memory_order_relaxed);
		}

	private:

		std::thread detectThread;

		std::atomic<bool> desktopObscured;

		std::atomic<bool> running;

		uint32_t w;

		uint32_t h;

		//用Spy++查看了下，我的win11系统是这个结构，目前用着没太大的问题
		bool isDesktop(const LONG x, const LONG y) const
		{
			const HWND hWnd = WindowFromPoint({ x, y });

			if (!hWnd || !IsWindowVisible(hWnd))
			{
				return true;
			}

			wchar_t className[256];

			GetClassNameW(hWnd, className, 256);

			bool isDesktop = ((_wcsicmp(className, L"Progman") == 0) || (_wcsicmp(className, L"WorkerW") == 0) || (hWnd == GetDesktopWindow()));

			if (!isDesktop && _wcsicmp(className, L"SysListView32") == 0)
			{
				const HWND parent = GetParent(hWnd);

				if (parent)
				{
					GetClassNameW(parent, className, 256);

					if (_wcsicmp(className, L"SHELLDLL_DefView") == 0)
					{
						const HWND root = GetAncestor(hWnd, GA_ROOT);

						if (root)
						{
							GetClassNameW(root, className, 256);

							if (_wcsicmp(className, L"Progman") == 0 || _wcsicmp(className, L"WorkerW") == 0)
							{
								isDesktop = true;
							}
						}
					}
				}
			}

			return isDesktop;
		}

		//AI生成的，后续可能要改改，目前用着没太大问题
		bool detectPoints() const
		{
			const uint32_t taskBarHeight = h / 9;
			const uint32_t topMargin = h / 24;
			const uint32_t bottomBoundary = h - taskBarHeight - 4;

			// =======================
			// 1. Y 轴：2-4-2 分布（修正版）
			// =======================
			const uint32_t vRange = bottomBoundary - topMargin;

			uint32_t yPos[8];

			// 上 2 点（均匀分布在 20%区域）
			for (uint32_t i = 0; i < 2; ++i)
			{
				yPos[i] = topMargin +
					vRange * 2 / 10 * (i * 2 + 1) / 4;
			}

			// 中 4 点（均匀分布在 60%区域）
			for (uint32_t i = 0; i < 4; ++i)
			{
				yPos[2 + i] = topMargin +
					vRange * 2 / 10 +
					vRange * 6 / 10 * (i * 2 + 1) / 8;
			}

			// 下 2 点（均匀分布在 20%区域）
			for (uint32_t i = 0; i < 2; ++i)
			{
				yPos[6 + i] = topMargin +
					vRange * 8 / 10 +
					vRange * 2 / 10 * (i * 2 + 1) / 4;
			}

			// =======================
			// 2. X 轴：均匀分布（保留你的思路，优化写法）
			// =======================
			const uint32_t hMargin = w / 64;
			const uint32_t hRange = w - 2 * hMargin;

			uint32_t xPos[8];

			for (uint32_t i = 0; i < 8; ++i)
			{
				xPos[i] = hMargin +
					hRange * (i * 2 + 1) / 16;
			}

			// =======================
			// 3. 采样检测
			// =======================
			uint32_t covered = 0;

			const uint32_t total = 64u;

			for (int yi = 0; yi < 8; ++yi)
			{
				LONG y = static_cast<LONG>(yPos[yi]);

				for (int xi = 0; xi < 8; ++xi)
				{
					LONG x = static_cast<LONG>(xPos[xi]);

					if (!isDesktop(x, y))
					{
						covered++;
					}
				}
			}

			const float coverage = static_cast<float>(covered) / static_cast<float>(total);

			LOGENGINE(coverage);

			return coverage > 0.90f;
		}

		void detectLoop()
		{
			CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

			while (running)
			{
				desktopObscured.store(detectPoints(), std::memory_order_relaxed);

				//定时进行检测
				//其它时间接收消息
				const size_t waitUntil = GetTickCount64() + 700;

				while (running && GetTickCount64() < waitUntil)
				{
					const DWORD result = MsgWaitForMultipleObjectsEx(0, nullptr, 100, QS_POSTMESSAGE | QS_SENDMESSAGE, 0);

					if (result == WAIT_OBJECT_0)
					{
						MSG msg;

						while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
						{
							if (msg.message == WM_QUIT)
							{
								break;
							}
						}
					}
				}
			}

			CoUninitialize();
		}
	};

	bool isDesktopObscured()
	{
		static DesktopDetectThread detectThread;

		return detectThread.getDesktopObscured();
	}
}
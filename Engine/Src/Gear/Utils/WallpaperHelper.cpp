#include<Gear/Utils/WallpaperHelper.h>

#include<Gear/Utils/MainMonitor.h>

#include<Gear/Utils/Logger.h>

#include<cstdint>

namespace Gear::Utils::WallpaperHelper
{
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

	class ObscureDetectThread
	{
	public:

		ObscureDetectThread() :
			obscured(false), running(true)
		{
			RECT workArea;

			if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
			{
				taskBarHeight = static_cast<uint32_t>(MainMonitor::getHeight() - workArea.bottom);
			}
			else
			{
				taskBarHeight = 72u;
			}

			LOGENGINE(L"任务栏高度", taskBarHeight);

			detectThread = std::thread(&ObscureDetectThread::detectLoop, this);
		}

		~ObscureDetectThread()
		{
			running.store(false);

			detectThread.join();
		}

		bool getObscured() const
		{
			return obscured.load(std::memory_order_relaxed);
		}

	private:

		std::thread detectThread;

		std::atomic<bool> obscured;

		std::atomic<bool> running;

		uint32_t taskBarHeight;

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
			const float marginRatio = 90.0f / 1600.0f;

			const float topY_f = MainMonitor::getHeight() * marginRatio;

			const float bottomBoundary_f = MainMonitor::getHeight() - taskBarHeight - MainMonitor::getHeight() * marginRatio;

			const float vRange_f = bottomBoundary_f - topY_f;

			uint32_t yPos[8];

			for (int i = 0; i < 8; ++i)
			{
				float y = topY_f + (vRange_f * static_cast<float>(i)) / 7.0f;

				yPos[i] = static_cast<uint32_t>(y + 0.5f);
			}

			// 确保端点精度
			yPos[0] = static_cast<uint32_t>(topY_f + 0.5f);

			yPos[7] = static_cast<uint32_t>(bottomBoundary_f + 0.5f);

			const float xMarginRatio = 90.0f / 2560.0f;         // 水平边距比例

			const float leftX_f = MainMonitor::getWidth() * xMarginRatio;

			const float rightX_f = MainMonitor::getWidth() - MainMonitor::getWidth() * xMarginRatio;

			const float hRange_f = rightX_f - leftX_f;

			uint32_t xPos[8];

			for (int i = 0; i < 8; ++i)
			{
				float x = leftX_f + (hRange_f * static_cast<float>(i)) / 7.0f;

				xPos[i] = static_cast<uint32_t>(x + 0.5f);
			}

			xPos[0] = static_cast<uint32_t>(leftX_f + 0.5f);

			xPos[7] = static_cast<uint32_t>(rightX_f + 0.5f);

			// =======================
			// 3. 采样检测
			// =======================
			uint32_t covered = 0;

			const uint32_t total = _countof(xPos) * _countof(yPos);

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

			//LOGENGINE(coverage);

			return coverage > 0.96f;
		}

		void detectLoop()
		{
			//定时进行检测
			while (running)
			{
				obscured.store(detectPoints(), std::memory_order_relaxed);

				Sleep(static_cast<DWORD>(obscureCheckInterval));
			}
		}
	};

	UniquePtr<ObscureDetectThread> detectThread;

	bool isDesktopObscured()
	{
		return detectThread->getObscured();
	}

	void initialize()
	{
		detectThread = makeUnique<ObscureDetectThread>();
	}

	void release()
	{
		detectThread.reset();
	}
}
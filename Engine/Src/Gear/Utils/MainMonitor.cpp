#include<Gear/Utils/MainMonitor.h>

#include<Gear/Utils/Internal/MainMonitorInternal.h>

#include<Gear/Utils/Logger.h>

#include<Windows.h>

namespace Gear::Utils::MainMonitor
{
	namespace Internal
	{
		struct MainMonitorImpl
		{
			void getSettings();

			uint32_t width;

			uint32_t height;

			uint32_t refreshRate;

			float scale;

		} impl;

		void MainMonitorImpl::getSettings()
		{
			HMONITOR monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);

			MONITORINFOEX monitorInfo;
			monitorInfo.cbSize = sizeof(MONITORINFOEX);

			GetMonitorInfo(monitor, &monitorInfo);

			DEVMODE devMode;
			devMode.dmSize = sizeof(DEVMODE);
			devMode.dmDriverExtra = 0;

			if (!EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode))
			{
				LOGERROR(L"无法获取主监视器信息");
			}

			width = devMode.dmPelsWidth;

			height = devMode.dmPelsHeight;

			if (devMode.dmDisplayFrequency == 0 || devMode.dmDisplayFrequency == 1)
			{
				refreshRate = 60u;
			}
			else
			{
				refreshRate = devMode.dmDisplayFrequency;
			}

			scale = static_cast<float>(devMode.dmLogPixels) / 96.f;

			LOGENGINE(L"以下是主监视器信息");

			LOGENGINE(L"主监视器宽", width);

			LOGENGINE(L"主监视器高", height);

			LOGENGINE(L"主监视器刷新率", refreshRate);

			LOGENGINE(L"主监视器缩放比率", scale);
		}

		void getSettings()
		{
			impl.getSettings();
		}
	}

	uint32_t getWidth()
	{
		return Internal::impl.width;
	}

	uint32_t getHeight()
	{
		return Internal::impl.height;
	}

	uint32_t getRefreshRate()
	{
		return Internal::impl.refreshRate;
	}

	float getScale()
	{
		return Internal::impl.scale;
	}
}
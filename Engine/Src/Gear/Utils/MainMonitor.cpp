#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <Gear/Utils/MainMonitor.h>

#include <Gear/Utils/Internal/MainMonitorInternal.h>

#include <Gear/Utils/Log.h>

#include <Windows.h>

namespace Gear::Utils::MainMonitor
{
	struct MainMonitorImpl
	{
		uint32_t width;

		uint32_t height;

		uint32_t refreshRate;

		float scale;

	} impl;

	namespace Internal
	{
		void getCurrentSettings()
		{
			HMONITOR monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);

			MONITORINFOEX monitorInfo;
			monitorInfo.cbSize = sizeof(MONITORINFOEX);

			GetMonitorInfo(monitor, &monitorInfo);

			DEVMODE devMode;
			devMode.dmSize = sizeof(DEVMODE);
			devMode.dmDriverExtra = 0;

			if (!EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode))
			{
				THROWLOG(LOGERROR() << "无法获取主监视器信息");
			}

			impl.width = devMode.dmPelsWidth;

			impl.height = devMode.dmPelsHeight;

			if (devMode.dmDisplayFrequency == 0 || devMode.dmDisplayFrequency == 1)
			{
				impl.refreshRate = 60u;
			}
			else
			{
				impl.refreshRate = devMode.dmDisplayFrequency;
			}

			impl.scale = static_cast<float>(devMode.dmLogPixels) / 96.f;

			LOGENGINE() << "以下是主监视器信息";

			LOGENGINE() << "主监视器宽" << impl.width;

			LOGENGINE() << "主监视器高" << impl.height;

			LOGENGINE() << "主监视器刷新率" << impl.refreshRate;

			LOGENGINE() << "主监视器缩放比率" << impl.scale;
		}
	}

	uint32_t getWidth()
	{
		return impl.width;
	}

	uint32_t getHeight()
	{
		return impl.height;
	}

	uint32_t getRefreshRate()
	{
		return impl.refreshRate;
	}

	float getScale()
	{
		return impl.scale;
	}
}

#include<Gear/Utils/MainMonitor.h>

#include<Gear/Utils/Internal/MainMonitorInternal.h>

#include<Gear/Utils/Logger.h>

#include<Windows.h>

namespace Gear::Utils::MainMonitor
{
	namespace Internal
	{
		uint32_t monitorWidth;

		uint32_t monitorHeight;

		uint32_t monitorRefreshRate;

		float monitorScale;

		void getSettings()
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

			monitorWidth = devMode.dmPelsWidth;

			monitorHeight = devMode.dmPelsHeight;

			if (devMode.dmDisplayFrequency == 0 || devMode.dmDisplayFrequency == 1)
			{
				monitorRefreshRate = 60u;
			}
			else
			{
				monitorRefreshRate = devMode.dmDisplayFrequency;
			}

			monitorScale = static_cast<float>(devMode.dmLogPixels) / 96.f;

			LOGENGINE(L"以下是主监视器信息");

			LOGENGINE(L"主监视器宽", monitorWidth);

			LOGENGINE(L"主监视器高", monitorHeight);

			LOGENGINE(L"主监视器刷新率", monitorRefreshRate);

			LOGENGINE(L"主监视器缩放比率", monitorScale);
		}
	}

	uint32_t getWidth()
	{
		return Internal::monitorWidth;
	}

	uint32_t getHeight()
	{
		return Internal::monitorHeight;
	}

	uint32_t getRefreshRate()
	{
		return Internal::monitorRefreshRate;
	}

	float getScale()
	{
		return Internal::monitorScale;
	}
}
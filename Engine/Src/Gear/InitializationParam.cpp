#include<Gear/InitializationParam.h>

namespace Gear
{
	InitializationParam InitializationParam::RealTime(const uint32_t width, const uint32_t height, const std::wstring& title, const bool enableImGuiSurface)
	{
		InitializationParam param = {};

		param.usage = EngineUsage::REALTIMERENDER;

		param.title = title;

		param.realTimeRender.width = width;

		param.realTimeRender.height = height;

		param.realTimeRender.enableImGuiSurface = enableImGuiSurface;

		return param;
	}

	InitializationParam InitializationParam::VideoRender(const uint32_t width, const uint32_t height, const uint32_t second)
	{
		InitializationParam param = {};

		param.usage = EngineUsage::VIDEORENDER;

		param.title = L"Video Render";

		param.videoRender.width = width;

		param.videoRender.height = height;

		param.videoRender.second = second;

		return param;
	}

	InitializationParam InitializationParam::Wallpaper()
	{
		InitializationParam param = {};

		param.usage = EngineUsage::WALLPAPER;

		param.title = L"Wallpaper";

		return param;
	}
}

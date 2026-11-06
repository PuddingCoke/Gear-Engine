#include <Gear/InitializationParam.h>

namespace Gear
{
	InitializationParam InitializationParam::RealTime(const uint32_t width,
	                                                  const uint32_t height,
	                                                  const std::wstring& title,
	                                                  const bool enableImGuiSurface)
	{
		InitializationParam param = {};

		param.usage = EngineUsage::REALTIMERENDER;

		param.title = title;

		param.width = width;

		param.height = height;

		param.enableImGuiSurface = enableImGuiSurface;

		return param;
	}

	InitializationParam InitializationParam::VideoRender(const uint32_t width,
	                                                     const uint32_t height,
	                                                     const uint32_t seconds,
	                                                     const bool hardWareEncode,
	                                                     const uint32_t maxBFrames)
	{
		InitializationParam param = {};

		param.usage = EngineUsage::VIDEORENDER;

		param.title = L"Video Render";

		param.width = width;

		param.height = height;

		param.enableImGuiSurface = false;

		param.videoRender.seconds = seconds;

		param.videoRender.hardwareEncode = hardWareEncode;

		param.videoRender.maxBFrames = maxBFrames;

		return param;
	}

	InitializationParam InitializationParam::Wallpaper(const int64_t targetFrameRate)
	{
		InitializationParam param = {};

		param.usage = EngineUsage::WALLPAPER;

		param.title = L"Wallpaper";

		param.width = 0u; //由引擎设置

		param.height = 0u; //由引擎设置

		param.enableImGuiSurface = false;

		param.wallpaper.targetFrameRate = targetFrameRate;

		return param;
	}
}

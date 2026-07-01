#pragma once

#ifndef _GEAR_INITIALIZATIONPARAM_H_
#define _GEAR_INITIALIZATIONPARAM_H_

#include<string>

namespace Gear
{
	struct InitializationParam
	{
		//使用nsight调试前请关闭ImGui，否则程序会莫名崩溃
		static InitializationParam RealTime(const uint32_t width, const uint32_t height, const std::wstring& title, const bool enableImGuiSurface = true);

		static InitializationParam VideoRender(const uint32_t width, const uint32_t height, const uint32_t second = 60u);

		static InitializationParam Wallpaper();

		enum class EngineUsage
		{
			REALTIMERENDER,
			VIDEORENDER,
			WALLPAPER
		}usage;

		std::wstring title;

		struct RealTimeRenderParam
		{
			uint32_t width;

			uint32_t height;

			bool enableImGuiSurface;
		};

		struct VideoRenderParam
		{
			uint32_t width;

			uint32_t height;

			uint32_t second;
		};

		union
		{
			RealTimeRenderParam realTimeRender;

			VideoRenderParam videoRender;
		};

	};
}

#endif // !_GEAR_INITIALIZATIONPARAM_H_
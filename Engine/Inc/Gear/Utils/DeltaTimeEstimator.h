#pragma once

#ifndef _GEAR_UTILS_DELTATIMEESTIMATOR_H_
#define _GEAR_UTILS_DELTATIMEESTIMATOR_H_

#include<cstdint>

namespace Gear::Utils
{
	/// <summary>
	/// 不知道为什么，以前在开发OpenGL的时候有时候画面会突然卡顿一下，明明是顶着显示器的刷新率运行的，而且GPU占用率也不高。
	/// 开发D3D11的时候，我不记得是否发生过了，在刚开始开发D3D12的时候也遇到了这个问题。
	/// 去网上搜了下发现别人也遇到了，解决方法是利用历史帧时间来缓和突然卡顿的影响，也是有点无语。
	/// </summary>
	class DeltaTimeEstimator
	{
	public:

		DeltaTimeEstimator(const DeltaTimeEstimator&) = delete;

		void operator=(const DeltaTimeEstimator&) = delete;

		DeltaTimeEstimator();

		~DeltaTimeEstimator();

		float getDeltaTime(const float lastDeltaTime);

	private:

		static constexpr float lerpFactor = 0.065f;

		static constexpr uint32_t numRecord = 11;

		static constexpr uint32_t numDiscard = 2;

		float historyDeltatime[numRecord];

		float sortedDeltaTime[numRecord];

		uint32_t historyDeltaTimeIndex;

		bool populated;

	};
}

#endif // !_GEAR_UTILS_DELTATIMEESTIMATOR_H_
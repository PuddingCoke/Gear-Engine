#pragma once

#ifndef _GEAR_UTILS_TIMER_H_
#define _GEAR_UTILS_TIMER_H_

namespace Gear::Utils
{
	class Timer
	{
	public:

		Timer();

		Timer(const float timeLimit);

		bool update(const float dt);

		void restart();

		void setTimeLimit(const float timeLimit);

		float getTimeLimit() const;

		float getCurrentTime() const;

		float getProgressRatio() const;

	private:

		float timeLimit;

		float currrentTime;

	};
}

#endif // !_GEAR_UTILS_TIMER_H_
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

	private:

		float timeLimit;

		float curTime;

	};
}

#endif // !_GEAR_UTILS_TIMER_H_
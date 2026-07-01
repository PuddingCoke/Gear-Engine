#include<Gear/Utils/Timer.h>

#include<Gear/Utils/Math.h>

namespace Gear::Utils
{
	Timer::Timer() :
		timeLimit(0), currrentTime(0.f)
	{

	}

	Timer::Timer(const float timeLimit) :
		timeLimit(timeLimit), currrentTime(0.f)
	{

	}

	bool Timer::update(const float dt)
	{
		if (currrentTime >= timeLimit)
		{
			currrentTime -= timeLimit;

			return true;
		}

		currrentTime += dt;

		return false;
	}

	void Timer::restart()
	{
		currrentTime = 0.f;
	}

	void Timer::setTimeLimit(const float timeLimit)
	{
		this->timeLimit = timeLimit;
	}

	float Timer::getTimeLimit() const
	{
		return timeLimit;
	}

	float Timer::getCurrentTime() const
	{
		return currrentTime;
	}

	float Timer::getProgressRatio() const
	{
		return Math::saturate(currrentTime / timeLimit);
	}
}
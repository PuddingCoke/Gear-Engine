#include<Gear/Input/Event.h>

namespace Gear::Input
{
	Event::Event() :
		idx(0)
	{
	}

	void Event::operator-=(const uint64_t id)
	{
		{
			std::lock_guard<std::mutex> lockGuard(containerMutex);

			functions.erase(id);
		}
	}

	uint64_t Event::operator+=(const std::function<void(void)>& func)
	{
		uint64_t retIndex;

		{
			std::lock_guard<std::mutex> lockGuard(containerMutex);

			retIndex = idx++;

			functions.emplace(retIndex, func);
		}

		return retIndex;
	}

	void Event::operator()()
	{
		for (auto& i : functions)
		{
			i.second();
		}
	}
}
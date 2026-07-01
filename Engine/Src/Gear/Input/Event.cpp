#include<Gear/Input/Event.h>

namespace Gear::Input
{
	Event::Event() :
		idx(0ull)
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
		const uint64_t retIndex = idx.fetch_add(1ull, std::memory_order_relaxed);

		{
			std::lock_guard<std::mutex> lockGuard(containerMutex);

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
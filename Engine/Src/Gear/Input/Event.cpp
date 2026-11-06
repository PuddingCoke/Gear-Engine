#include <Gear/Input/Event.h>

namespace Gear::Input
{
	Event::Event() : idx(0ull), triggerCount(0u)
	{
	}

	void Event::operator-=(const uint64_t id)
	{
		{
			std::lock_guard<std::mutex> lockGuard(containerMutex);

			functions.erase(id);
		}
	}

	uint64_t Event::operator+=(const std::function<void(const uint32_t)>& func)
	{
		const uint64_t retIndex = idx.fetch_add(1ull, std::memory_order_relaxed);

		{
			std::lock_guard<std::mutex> lockGuard(containerMutex);

			functions.emplace(retIndex, func);
		}

		return retIndex;
	}

	void Event::trigger() const
	{
		if (triggerCount)
		{
			for (auto& i : functions)
			{
				i.second(triggerCount);
			}
		}
	}

	uint32_t Event::getTriggerCount() const
	{
		return triggerCount;
	}

	void Event::increaseTriggerCount()
	{
		triggerCount++;
	}

	void Event::resetTriggerCount()
	{
		triggerCount = 0u;
	}

}

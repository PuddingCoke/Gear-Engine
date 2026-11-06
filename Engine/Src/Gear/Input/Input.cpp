#include <Gear/Input/Internal/InputInternal.h>

#include <vector>

namespace Gear::Input::Internal
{

	std::vector<Event*> eventTriggerList = std::vector<Event*>();

	void pushToEventTriggerList(Event& event)
	{
		if (!event.getTriggerCount())
		{
			eventTriggerList.push_back(&event);
		}

		event.increaseTriggerCount();
	}

	void clearEventTriggerList()
	{
		if (eventTriggerList.size())
		{
			for (Event* const eventPtr : eventTriggerList)
			{
				eventPtr->resetTriggerCount();
			}

			eventTriggerList.clear();
		}
	}

	void triggerEvents()
	{
		if (eventTriggerList.size())
		{
			for (Event* const eventPtr : eventTriggerList)
			{
				eventPtr->trigger();
			}
		}
	}

}

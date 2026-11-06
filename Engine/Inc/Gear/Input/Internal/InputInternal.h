#pragma once

#ifndef _GEAR_INPUT_INTERNAL_H_
#define _GEAR_INPUT_INTERNAL_H_

#include <Gear/Input/Event.h>

namespace Gear::Input::Internal
{
	//添加事件到事件触发列表并使其触发计数自增1
	void pushToEventTriggerList(Event& event);

	//重置所有事件的触发计数到0并清空事件触发列表
	void clearEventTriggerList();

	void triggerEvents();

}

#endif // !_GEAR_INPUT_INTERNAL_H_

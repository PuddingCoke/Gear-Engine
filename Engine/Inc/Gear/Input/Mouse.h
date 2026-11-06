#pragma once

#ifndef _GEAR_INPUT_MOUSE_H_
#define _GEAR_INPUT_MOUSE_H_

#include <cstdint>

#include <functional>

namespace Gear::Input::Mouse
{
	//横坐标 左侧为 0
	float getX();

	//纵坐标 底部为 0
	float getY();

	//横坐标的变化量 向右为正
	float getDeltaX();

	//纵坐标的变化量 向上为正
	float getDeltaY();

	//向前 +1 向后 -1
	float getWheelDelta();

	//获取左键按下状态
	bool getLeftDown();

	//获取右键按下状态
	bool getRightDown();

	//获取移动触发次数
	uint32_t getMoveTriggerCount();

	//获取左键按下触发次数
	uint32_t getLeftDownTriggerCount();

	//获取右键按下触发次数
	uint32_t getRightDownTriggerCount();

	//获取左键弹起触发次数
	uint32_t getLeftUpTriggerCount();

	//获取右键弹起触发次数
	uint32_t getRightUpTriggerCount();

	//获取滚轮滚动触发次数
	uint32_t getScrollTriggerCount();

	//这一帧是否移动
	bool getOnMove();

	//这一帧是否按下左键
	bool getOnLeftDown();

	//这一帧是否按下右键
	bool getOnRightDown();

	//这一帧是否弹起左键
	bool getOnLeftUp();

	//这一帧是否弹起右键
	bool getOnRightUp();

	//这一帧滚轮是否滚动
	bool getOnScroll();

	uint64_t addMoveEvent(const std::function<void(const uint32_t triggerCount)>& func);

	uint64_t addLeftDownEvent(const std::function<void(const uint32_t triggerCount)>& func);

	uint64_t addRightDownEvent(const std::function<void(const uint32_t triggerCount)>& func);

	uint64_t addLeftUpEvent(const std::function<void(const uint32_t triggerCount)>& func);

	uint64_t addRightUpEvent(const std::function<void(const uint32_t triggerCount)>& func);

	uint64_t addScrollEvent(const std::function<void(const uint32_t triggerCount)>& func);

	void removeMoveEvent(const uint64_t id);

	void removeLeftDownEvent(const uint64_t id);

	void removeRightDownEvent(const uint64_t id);

	void removeLeftUpEvent(const uint64_t id);

	void removeRightUpEvent(const uint64_t id);

	void removeScrollEvent(const uint64_t id);
}

#endif // !_GEAR_INPUT_MOUSE_H_

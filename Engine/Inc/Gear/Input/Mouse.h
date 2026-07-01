#pragma once

#ifndef _GEAR_INPUT_MOUSE_H_
#define _GEAR_INPUT_MOUSE_H_

#include<cstdint>

#include<functional>

namespace Gear::Input::Mouse
{
	float getX();

	float getY();

	float getDX();

	float getDY();

	float getWheelDelta();

	bool getLeftDown();

	bool getRightDown();

	bool onMove();

	bool onLeftDown();

	bool onRightDown();

	bool onScroll();

	uint64_t addMoveEvent(const std::function<void(void)>& func);

	uint64_t addLeftDownEvent(const std::function<void(void)>& func);

	uint64_t addRightDownEvent(const std::function<void(void)>& func);

	uint64_t addLeftUpEvent(const std::function<void(void)>& func);

	uint64_t addRightUpEvent(const std::function<void(void)>& func);

	uint64_t addScrollEvent(const std::function<void(void)>& func);

	void removeMoveEvent(const uint64_t id);

	void removeLeftDownEvent(const uint64_t id);

	void removeRightDownEvent(const uint64_t id);

	void removeLeftUpEvent(const uint64_t id);

	void removeRightUpEvent(const uint64_t id);

	void removeScrollEvent(const uint64_t id);
}

#endif // !_GEAR_INPUT_MOUSE_H_
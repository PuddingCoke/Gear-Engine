#pragma once

#ifndef _GEAR_INPUT_MOUSE_INTERNAL_H_
#define _GEAR_INPUT_MOUSE_INTERNAL_H_

namespace Gear::Input::Mouse::Internal
{
	void resetDeltaValue();

	void pressLeft();

	void pressRight();

	void releaseLeft();

	void releaseRight();

	void scroll(const float delta);

	void move(const float curX, const float curY);
}

#endif // !_GEAR_INPUT_MOUSE_INTERNAL_H_

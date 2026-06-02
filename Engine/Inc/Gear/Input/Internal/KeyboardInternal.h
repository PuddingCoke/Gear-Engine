#pragma once

#ifndef _GEAR_INPUT_KEYBOARD_INTERNAL_H_
#define _GEAR_INPUT_KEYBOARD_INTERNAL_H_

namespace Gear::Input::Keyboard::Internal
{
	void resetDeltaValue();

	void pressKey(const Key key);

	void releaseKey(const Key key);
}

#endif // !_GEAR_INPUT_KEYBOARD_INTERNAL_H_

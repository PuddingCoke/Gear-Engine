#pragma once

#ifndef _GEAR_INPUT_KEYBOARD_H_
#define _GEAR_INPUT_KEYBOARD_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <cstdint>

#include <functional>

#include <Windows.h>

namespace Gear::Input::Keyboard
{
	enum Key
	{
		Space = VK_SPACE,
		Apostrophe = VK_OEM_7,
		Comma = VK_OEM_COMMA,
		Minus = VK_OEM_MINUS,
		Period = VK_OEM_PERIOD,
		Slash = VK_OEM_2,
		Num0 = 0x30,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9 = 0x39,
		Semicolon = VK_OEM_1,
		Equal = VK_OEM_PLUS,
		A = 0x41,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z = 0x5A,
		LeftBracket = VK_OEM_4,
		Backslash = VK_OEM_5,
		RightBracket = VK_OEM_6,
		GraveAccent = VK_OEM_3,
		Escape = VK_ESCAPE,
		Enter = VK_RETURN,
		Tab = VK_TAB,
		Backspace = VK_BACK,
		Insert = VK_INSERT,
		Delete = VK_DELETE,
		Right = VK_RIGHT,
		Left = VK_LEFT,
		Down = VK_DOWN,
		Up = VK_UP,
		PageUp = VK_PRIOR,
		PageDown = VK_NEXT,
		Home = VK_HOME,
		End = VK_END,
		F1 = VK_F1,
		F2 = VK_F2,
		F3 = VK_F3,
		F4 = VK_F4,
		F5 = VK_F5,
		F6 = VK_F6,
		F7 = VK_F7,
		F8 = VK_F8,
		F9 = VK_F9,
		F10 = VK_F10,
		F11 = VK_F11,
		F12 = VK_F12,
		Numpad0 = VK_NUMPAD0,
		Numpad1 = VK_NUMPAD1,
		Numpad2 = VK_NUMPAD2,
		Numpad3 = VK_NUMPAD3,
		Numpad4 = VK_NUMPAD4,
		Numpad5 = VK_NUMPAD5,
		Numpad6 = VK_NUMPAD6,
		Numpad7 = VK_NUMPAD7,
		Numpad8 = VK_NUMPAD8,
		Numpad9 = VK_NUMPAD9,
		NumpadDecimal = VK_DECIMAL,
		NumpadDivide = VK_DIVIDE,
		NumpadMultiply = VK_MULTIPLY,
		NumpadSubtract = VK_SUBTRACT,
		NumpadAdd = VK_ADD,
		Shift = VK_SHIFT,
		Control = VK_CONTROL,
		Alt = VK_MENU,
		LeftShift = VK_LSHIFT,
		LeftControl = VK_LCONTROL,
		LeftAlt = VK_LMENU,
		RightShift = VK_RSHIFT,
		RightControl = VK_RCONTROL,
		RightAlt = VK_RMENU,
		Menu = VK_APPS
	};

	bool getKeyDown(const Key key);

	uint32_t getKeyDownTriggerCount(const Key key);

	uint32_t getKeyUpTriggerCount(const Key key);

	bool onKeyDown(const Key key);

	bool onKeyUp(const Key key);

	uint64_t addKeyDownEvent(const Key key, const std::function<void(const uint32_t triggerCount)>& func);

	uint64_t addKeyUpEvent(const Key key, const std::function<void(const uint32_t triggerCount)>& func);

	void removeKeyDownEvent(const Key key, const uint64_t id);

	void removeKeyUpEvent(const Key key, const uint64_t id);
}

#endif // !_GEAR_INPUT_KEYBOARD_H_

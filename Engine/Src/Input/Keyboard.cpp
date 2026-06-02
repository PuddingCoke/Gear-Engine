#include<Gear/Input/Keyboard.h>

#include<Gear/Input/Internal/KeyboardInternal.h>

namespace
{
	struct KeyboardImpl
	{

		static constexpr size_t maxKey = 512;

		Gear::Input::Event keyDownEvents[maxKey] = {};

		Gear::Input::Event keyUpEvents[maxKey] = {};

		bool keyDownStates[maxKey] = {};

		bool onKeyDownStates[maxKey] = {};

		std::vector<uint32_t> onKeyDownClearList = std::vector<uint32_t>();

	} impl;
}

bool Gear::Input::Keyboard::getKeyDown(const Key key)
{
	return impl.keyDownStates[key];
}

bool Gear::Input::Keyboard::onKeyDown(const Key key)
{
	return impl.onKeyDownStates[key];
}

uint64_t Gear::Input::Keyboard::addKeyDownEvent(const Key key, const std::function<void(void)>& func)
{
	return impl.keyDownEvents[key] += func;
}

uint64_t Gear::Input::Keyboard::addKeyUpEvent(const Key key, const std::function<void(void)>& func)
{
	return impl.keyUpEvents[key] += func;
}

void Gear::Input::Keyboard::removeKeyDownEvent(const Key key, const uint64_t id)
{
	impl.keyDownEvents[key] -= id;
}

void Gear::Input::Keyboard::removeKeyUpEvent(const Key key, const uint64_t id)
{
	impl.keyUpEvents[key] -= id;
}

void Gear::Input::Keyboard::Internal::resetDeltaValue()
{
	if (impl.onKeyDownClearList.size())
	{
		for (uint32_t i = 0; i < impl.onKeyDownClearList.size(); i++)
		{
			const uint32_t idx = impl.onKeyDownClearList[i];

			impl.onKeyDownStates[idx] = false;
		}

		impl.onKeyDownClearList.clear();
	}
}

void Gear::Input::Keyboard::Internal::pressKey(const Key key)
{
	impl.keyDownStates[key] = true;

	impl.onKeyDownStates[key] = true;

	impl.onKeyDownClearList.emplace_back(key);

	impl.keyDownEvents[key]();
}

void Gear::Input::Keyboard::Internal::releaseKey(const Key key)
{
	impl.keyDownStates[key] = false;

	impl.keyUpEvents[key]();
}

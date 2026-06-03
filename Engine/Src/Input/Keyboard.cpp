#include<Gear/Input/Keyboard.h>

#include<Gear/Input/Internal/KeyboardInternal.h>

namespace Gear::Input::Keyboard
{
	namespace Internal
	{
		struct KeyboardImpl
		{

			static constexpr size_t maxKey = 512;

			Event keyDownEvents[maxKey] = {};

			Event keyUpEvents[maxKey] = {};

			bool keyDownStates[maxKey] = {};

			bool onKeyDownStates[maxKey] = {};

			std::vector<uint32_t> onKeyDownClearList = std::vector<uint32_t>();

		} impl;

		void resetDeltaValue()
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

		void pressKey(const Key key)
		{
			impl.keyDownStates[key] = true;

			impl.onKeyDownStates[key] = true;

			impl.onKeyDownClearList.emplace_back(key);

			impl.keyDownEvents[key]();
		}

		void releaseKey(const Key key)
		{
			impl.keyDownStates[key] = false;

			impl.keyUpEvents[key]();
		}

	}

	bool getKeyDown(const Key key)
	{
		return Internal::impl.keyDownStates[key];
	}

	bool onKeyDown(const Key key)
	{
		return Internal::impl.onKeyDownStates[key];
	}

	uint64_t addKeyDownEvent(const Key key, const std::function<void(void)>& func)
	{
		return Internal::impl.keyDownEvents[key] += func;
	}

	uint64_t addKeyUpEvent(const Key key, const std::function<void(void)>& func)
	{
		return Internal::impl.keyUpEvents[key] += func;
	}

	void removeKeyDownEvent(const Key key, const uint64_t id)
	{
		Internal::impl.keyDownEvents[key] -= id;
	}

	void removeKeyUpEvent(const Key key, const uint64_t id)
	{
		Internal::impl.keyUpEvents[key] -= id;
	}
}
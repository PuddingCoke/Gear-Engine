#include<Gear/Input/Mouse.h>

#include<Gear/Input/Internal/MouseInternal.h>

#include<Gear/Input/Event.h>

namespace Gear::Input::Mouse
{
	namespace Internal
	{
		struct MouseImpl
		{

			float x = 0.f;

			float y = 0.f;

			float dx = 0.f;

			float dy = 0.f;

			float wheelDelta = 0.f;

			bool leftDown = false;

			bool rightDown = false;

			bool onMoved = false;

			bool onLeftDowned = false;

			bool onRightDowned = false;

			bool onScrolled = false;

			Event moveEvent;

			Event leftDownEvent;

			Event rightDownEvent;

			Event leftUpEvent;

			Event rightUpEvent;

			Event scrollEvent;

		}impl;

		void resetDeltaValue()
		{
			Internal::impl.dx = 0;

			Internal::impl.dy = 0;

			Internal::impl.onMoved = false;

			Internal::impl.onLeftDowned = false;

			Internal::impl.onRightDowned = false;

			Internal::impl.onScrolled = false;
		}

		void pressLeft()
		{
			impl.leftDown = true;

			impl.onLeftDowned = true;

			impl.leftDownEvent();
		}

		void pressRight()
		{
			impl.rightDown = true;

			impl.onRightDowned = true;

			impl.rightDownEvent();
		}

		void releaseLeft()
		{
			impl.leftDown = false;

			impl.leftUpEvent();
		}

		void releaseRight()
		{
			impl.rightDown = false;

			impl.rightUpEvent();
		}

		void scroll(const float delta)
		{
			impl.wheelDelta = delta;

			impl.onScrolled = true;

			impl.scrollEvent();
		}

		void move(const float curX, const float curY)
		{
			impl.dx = curX - impl.x;

			impl.dy = curY - impl.y;

			impl.x = curX;

			impl.y = curY;

			impl.onMoved = true;

			impl.moveEvent();
		}
	}

	float getX()
	{
		return Internal::impl.x;
	}

	float getY()
	{
		return Internal::impl.y;
	}

	float getDX()
	{
		return Internal::impl.dx;
	}

	float getDY()
	{
		return Internal::impl.dy;
	}

	float getWheelDelta()
	{
		return Internal::impl.wheelDelta;
	}

	bool getLeftDown()
	{
		return Internal::impl.leftDown;
	}

	bool getRightDown()
	{
		return Internal::impl.rightDown;
	}

	bool onMove()
	{
		return Internal::impl.onMoved;
	}

	bool onLeftDown()
	{
		return Internal::impl.onLeftDowned;
	}

	bool onRightDown()
	{
		return Internal::impl.onRightDowned;
	}

	bool onScroll()
	{
		return Internal::impl.onScrolled;
	}

	uint64_t addMoveEvent(const std::function<void(void)>& func)
	{
		return Internal::impl.moveEvent += func;
	}

	uint64_t addLeftDownEvent(const std::function<void(void)>& func)
	{
		return Internal::impl.leftDownEvent += func;
	}

	uint64_t addRightDownEvent(const std::function<void(void)>& func)
	{
		return Internal::impl.rightDownEvent += func;
	}

	uint64_t addLeftUpEvent(const std::function<void(void)>& func)
	{
		return Internal::impl.leftUpEvent += func;
	}

	uint64_t addRightUpEvent(const std::function<void(void)>& func)
	{
		return Internal::impl.rightUpEvent += func;
	}

	uint64_t addScrollEvent(const std::function<void(void)>& func)
	{
		return Internal::impl.scrollEvent += func;
	}

	void removeMoveEvent(const uint64_t id)
	{
		Internal::impl.moveEvent -= id;
	}

	void removeLeftDownEvent(const uint64_t id)
	{
		Internal::impl.leftDownEvent -= id;
	}

	void removeRightDownEvent(const uint64_t id)
	{
		Internal::impl.rightDownEvent -= id;
	}

	void removeLeftUpEvent(const uint64_t id)
	{
		Internal::impl.leftUpEvent -= id;
	}

	void removeRightUpEvent(const uint64_t id)
	{
		Internal::impl.rightUpEvent -= id;
	}

	void removeScrollEvent(const uint64_t id)
	{
		Internal::impl.scrollEvent -= id;
	}
}


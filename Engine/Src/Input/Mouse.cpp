#include<Gear/Input/Mouse.h>

#include<Gear/Input/Internal/MouseInternal.h>

namespace
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

		Gear::Input::Event moveEvent;

		Gear::Input::Event leftDownEvent;

		Gear::Input::Event rightDownEvent;

		Gear::Input::Event leftUpEvent;

		Gear::Input::Event rightUpEvent;

		Gear::Input::Event scrollEvent;

	}impl;
}

float Gear::Input::Mouse::getX()
{
	return impl.x;
}

float Gear::Input::Mouse::getY()
{
	return impl.y;
}

float Gear::Input::Mouse::getDX()
{
	return impl.dx;
}

float Gear::Input::Mouse::getDY()
{
	return impl.dy;
}

float Gear::Input::Mouse::getWheelDelta()
{
	return impl.wheelDelta;
}

bool Gear::Input::Mouse::getLeftDown()
{
	return impl.leftDown;
}

bool Gear::Input::Mouse::getRightDown()
{
	return impl.rightDown;
}

bool Gear::Input::Mouse::onMove()
{
	return impl.onMoved;
}

bool Gear::Input::Mouse::onLeftDown()
{
	return impl.onLeftDowned;
}

bool Gear::Input::Mouse::onRightDown()
{
	return impl.onRightDowned;
}

bool Gear::Input::Mouse::onScroll()
{
	return impl.onScrolled;
}

uint64_t Gear::Input::Mouse::addMoveEvent(const std::function<void(void)>& func)
{
	return impl.moveEvent += func;
}

uint64_t Gear::Input::Mouse::addLeftDownEvent(const std::function<void(void)>& func)
{
	return impl.leftDownEvent += func;
}

uint64_t Gear::Input::Mouse::addRightDownEvent(const std::function<void(void)>& func)
{
	return impl.rightDownEvent += func;
}

uint64_t Gear::Input::Mouse::addLeftUpEvent(const std::function<void(void)>& func)
{
	return impl.leftUpEvent += func;
}

uint64_t Gear::Input::Mouse::addRightUpEvent(const std::function<void(void)>& func)
{
	return impl.rightUpEvent += func;
}

uint64_t Gear::Input::Mouse::addScrollEvent(const std::function<void(void)>& func)
{
	return impl.scrollEvent += func;
}

void Gear::Input::Mouse::removeMoveEvent(const uint64_t id)
{
	impl.moveEvent -= id;
}

void Gear::Input::Mouse::removeLeftDownEvent(const uint64_t id)
{
	impl.leftDownEvent -= id;
}

void Gear::Input::Mouse::removeRightDownEvent(const uint64_t id)
{
	impl.rightDownEvent -= id;
}

void Gear::Input::Mouse::removeLeftUpEvent(const uint64_t id)
{
	impl.leftUpEvent -= id;
}

void Gear::Input::Mouse::removeRightUpEvent(const uint64_t id)
{
	impl.rightUpEvent -= id;
}

void Gear::Input::Mouse::removeScrollEvent(const uint64_t id)
{
	impl.scrollEvent -= id;
}

void Gear::Input::Mouse::Internal::resetDeltaValue()
{
	impl.dx = 0;

	impl.dy = 0;

	impl.onMoved = false;

	impl.onLeftDowned = false;

	impl.onRightDowned = false;

	impl.onScrolled = false;
}

void Gear::Input::Mouse::Internal::pressLeft()
{
	impl.leftDown = true;

	impl.onLeftDowned = true;

	impl.leftDownEvent();
}

void Gear::Input::Mouse::Internal::pressRight()
{
	impl.rightDown = true;

	impl.onRightDowned = true;

	impl.rightDownEvent();
}

void Gear::Input::Mouse::Internal::releaseLeft()
{
	impl.leftDown = false;

	impl.leftUpEvent();
}

void Gear::Input::Mouse::Internal::releaseRight()
{
	impl.rightDown = false;

	impl.rightUpEvent();
}

void Gear::Input::Mouse::Internal::scroll(const float delta)
{
	impl.wheelDelta = delta;

	impl.onScrolled = true;

	impl.scrollEvent();
}

void Gear::Input::Mouse::Internal::move(const float curX, const float curY)
{
	impl.dx = curX - impl.x;

	impl.dy = curY - impl.y;

	impl.x = curX;

	impl.y = curY;

	impl.onMoved = true;

	impl.moveEvent();
}


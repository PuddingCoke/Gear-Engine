#pragma once

#ifndef _GEAR_INPUT_EVENT_H_
#define _GEAR_INPUT_EVENT_H_

#include<unordered_map>

#include<functional>

#include<cstdint>

#include<mutex>

#include<atomic>

namespace Gear::Input
{
	class Event
	{
	public:

		Event();

		void operator-=(const uint64_t id);

		uint64_t operator+=(const std::function<void(void)>& func);

		void operator()();

	private:

		std::atomic<uint64_t> idx;

		std::unordered_map<uint64_t, std::function<void(void)>> functions;

		std::mutex containerMutex;

	};
}

#endif // !_GEAR_INPUT_EVENT_H_
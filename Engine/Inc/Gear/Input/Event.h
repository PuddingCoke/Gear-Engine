#pragma once

#ifndef _GEAR_INPUT_EVENT_H_
#define _GEAR_INPUT_EVENT_H_

#include <unordered_map>

#include <functional>

#include <cstdint>

#include <mutex>

#include <atomic>

namespace Gear::Input
{
	class Event
	{
	public:

		Event();

		void operator-=(const uint64_t id);

		uint64_t operator+=(const std::function<void(const uint32_t)>& func);

		void trigger() const;

		uint32_t getTriggerCount() const;

		void increaseTriggerCount();

		void resetTriggerCount();

	private:

		std::atomic<uint64_t> idx;

		std::unordered_map<uint64_t, std::function<void(const uint32_t)>> functions;

		std::mutex containerMutex;

		uint32_t triggerCount;
	};
}

#endif // !_GEAR_INPUT_EVENT_H_

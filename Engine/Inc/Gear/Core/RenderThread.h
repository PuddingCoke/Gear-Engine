#pragma once

#ifndef _GEAR_CORE_RENDERTHREAD_H_
#define _GEAR_CORE_RENDERTHREAD_H_

#include"RenderTask.h"

#include<mutex>

namespace Gear::Core
{
	class RenderThread
	{
	public:

		RenderThread(const std::function<void(RenderTask** renderTask)>& createFunc);

		~RenderThread();

		bool waitInitialized();

	private:

		void workerLoop();

		bool initialized;

		bool errorOccured;

		std::mutex taskMutex;

		std::condition_variable taskCondition;

		std::function<void(RenderTask** renderTask)> createFunc;

		//引用
		RenderTask* renderTask;

		std::thread renderThread;

	};

	template <typename First, typename... Rest>
	RenderThread* createRenderTaskAsync(const First& first, const Rest&... args)
	{
		using RenderTaskType = std::remove_pointer_t<std::remove_pointer_t<First>>;

		auto createFunc = [&](RenderTask** renderTask)
			{
				*renderTask = *first = new RenderTaskType(args...);
			};

		return new RenderThread(createFunc);
	}
}

#endif // !_GEAR_CORE_RENDERTHREAD_H_

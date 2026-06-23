#pragma once

#ifndef _GEAR_CORE_RENDERTHREAD_H_
#define _GEAR_CORE_RENDERTHREAD_H_

#include"RenderTask.h"

#include<mutex>

namespace Gear::Core
{
	CREATESAFETYPE(RenderThread);
	
	//渲染线程，它会执行与它关联的渲染任务的构造方法和帧任务方法
	class RenderThread
	{
	public:

		RenderThread(const std::function<void(RenderTask** renderTask)>& createFunc);

		~RenderThread();

		void beginTask();

		bool waitTask();

		//把RenderThread管理权移交给RenderTask
		void transferOwnerShip(UniquePtr<RenderThread> renderThread);

		std::string getErrorStr() const;

	private:

		void workerLoop();

		bool taskCompleted;

		bool errorOccured;

		bool isRunning;

		std::mutex taskMutex;

		std::condition_variable taskCondition;

		std::function<void(RenderTask** renderTask)> createFunc;

		//引用
		RenderTask* renderTask;

		std::thread renderThread;

		std::string errorStr;

	};

	//异步创建函数，原理很简单
	//先把RenderThread所有权移交给Game
	//待RenderTask初始化完毕后，Game调用RenderThread的transferOwnerShip把所有权移交给RenderTask
	template <typename First, typename... Rest>
	UniquePtr<RenderThread> createRenderTaskAsync(First& first, Rest&&... args)
	{
		using RenderTaskType = typename First::element_type;

		//这里有点危险，有时间再来想想该怎么改这里
		auto createFunc = [&](RenderTask** renderTask)
			{
				first = makeUnique<RenderTaskType>(std::forward<Rest>(args)...);

				*renderTask = first.get();
			};

		return makeUnique<RenderThread>(createFunc);
	}
}

#endif // !_GEAR_CORE_RENDERTHREAD_H_

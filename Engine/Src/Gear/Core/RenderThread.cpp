#include<Gear/Core/RenderThread.h>

#include<Gear/Core/Internal/RenderThreadLocalInternal.h>

#include<Gear/Core/RenderEngine.h>

namespace Gear::Core
{
	RenderThread::RenderThread(const std::function<void(RenderTask**)>& createFunc) :
		taskCompleted(false), errorOccured(false), isRunning(true), createFunc(createFunc), renderTask(nullptr)
	{
		renderThread = std::thread(&RenderThread::workerLoop, this);
	}

	RenderThread::~RenderThread()
	{
		isRunning = false;

		beginTask();

		if (renderThread.joinable())
		{
			renderThread.join();
		}
	}

	void RenderThread::beginTask()
	{
		{
			std::lock_guard<std::mutex> lockGuard(taskMutex);

			taskCompleted = false;
		}

		taskCondition.notify_one();
	}

	bool RenderThread::waitTask()
	{
		std::unique_lock<std::mutex> lock(taskMutex);

		taskCondition.wait(lock, [this]() {return taskCompleted; });

		return errorOccured;
	}

	void RenderThread::transferOwnerShip(UniquePtr<RenderThread> renderThread)
	{
		renderTask->renderThread = std::move(renderThread);
	}

	std::string RenderThread::getErrorStr() const
	{
		return errorStr;
	}

	void RenderThread::workerLoop()
	{
		//初始化线程局部资源
		RenderThreadLocal::Internal::InitializeToken renderThreadLocalToken;

		try
		{
			//开始创建RenderTask
			createFunc(&renderTask);

			RenderEngine::submitCommandList(renderTask->getCommandList());

			{
				std::lock_guard<std::mutex> lockGuard(taskMutex);

				taskCompleted = true;
			}

		}
		catch (const std::exception& e)
		{
			{
				std::lock_guard<std::mutex> lockGuard(taskMutex);

				taskCompleted = true;

				errorOccured = true;

				errorStr = e.what();
			}

			//通知主渲染线程子渲染线程创建完毕
			taskCondition.notify_one();

			return;
		}

		//通知主渲染线程子渲染线程创建完毕
		taskCondition.notify_one();

		try
		{
			//进入工作循环
			while (true)
			{
				{
					std::unique_lock<std::mutex> lock(taskMutex);

					taskCondition.wait(lock, [this]() {return !taskCompleted; });

					if (!isRunning)
					{
						break;
					}

					RenderThreadLocal::Internal::flushCopiedResources();

					renderTask->frameTask();

					taskCompleted = true;
				}

				//通知主渲染线程帧任务完成
				taskCondition.notify_one();
			}
		}
		catch (const std::exception& e)
		{
			{
				std::unique_lock<std::mutex> lock(taskMutex);

				taskCompleted = true;

				errorOccured = true;

				errorStr = e.what();
			}

			taskCondition.notify_one();

			return;
		}
	}
}
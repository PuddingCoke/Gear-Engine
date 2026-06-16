#include<Gear/Core/RenderThread.h>

#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/LocalDescriptorHeapInternal.h>

#include<Gear/Core/D3D12Core/Internal/DXCCompilerInternal.h>

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
		//这里一定要调用CoInitialize
		CoInitializeToken coInitializeToken;

		//初始化每个渲染线程独享的DXC编译器
		D3D12Core::DXCCompiler::Internal::InitializeToken dxcCompilerToken;

		//申请每个渲染线程独享的描述符堆
		LocalDescriptorHeap::Internal::InitializeToken localDescriptorHeapToken;

#ifdef _DEBUG
		try
		{
#endif // _DEBUG
			//开始创建RenderTask
			{
				std::lock_guard<std::mutex> lockGuard(taskMutex);

				createFunc(&renderTask);

				RenderEngine::submitCommandList(renderTask->getCommandList());

				taskCompleted = true;
			}
#ifdef _DEBUG
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
#endif // _DEBUG

		//通知主渲染线程子渲染线程创建完毕
		taskCondition.notify_one();

#ifdef _DEBUG
		try
		{
#endif // _DEBUG
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

					renderTask->frameTask();

					taskCompleted = true;
				}

				//通知主渲染线程帧任务完成
				taskCondition.notify_one();
			}
#ifdef _DEBUG
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
#endif // _DEBUG
	}
}
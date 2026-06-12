#include<Gear/Core/RenderThread.h>

#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/LocalDescriptorHeapInternal.h>

#include<Gear/Core/D3D12Core/Internal/DXCCompilerInternal.h>

namespace Gear::Core
{
	RenderThread::RenderThread(const std::function<void(RenderTask**)>& createFunc) :
		initialized(false), createFunc(createFunc), renderTask(nullptr), errorOccured(false)
	{
		renderThread = std::thread(&RenderThread::workerLoop, this);
	}

	RenderThread::~RenderThread()
	{
		if (renderThread.joinable())
		{
			renderThread.join();
		}
	}

	bool RenderThread::waitInitialized()
	{
		std::unique_lock<std::mutex> lock(taskMutex);

		taskCondition.wait(lock, [this]() {return initialized; });

		return errorOccured;
	}

	void RenderThread::transferOwnerShip(UniquePtr<RenderThread> renderThread)
	{
		renderTask->renderThread = std::move(renderThread);
	}

	void RenderThread::workerLoop()
	{
		CoInitializeToken coInitializeToken;

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

				initialized = true;
			}
#ifdef _DEBUG
		}
		catch (const std::exception&)
		{
			{
				std::lock_guard<std::mutex> lockGuard(taskMutex);

				initialized = true;

				errorOccured = true;
			}

			//通知主渲染线程子渲染线程创建完毕
			taskCondition.notify_one();

			return;
		}
#endif // _DEBUG

		//通知主渲染线程子渲染线程创建完毕
		taskCondition.notify_one();

		//进入工作循环
		//由主渲染线程来调度
		renderTask->workerLoop();
	}
}
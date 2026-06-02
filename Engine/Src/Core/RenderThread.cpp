#include<Gear/Core/RenderThread.h>

#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/LocalDescriptorHeapInternal.h>

Gear::Core::RenderThread::RenderThread(const std::function<void(RenderTask**)>& createFunc) :
	initialized(false), createFunc(createFunc), renderTask(nullptr), errorOccured(false)
{
	renderThread = std::thread(&RenderThread::workerLoop, this);
}

Gear::Core::RenderThread::~RenderThread()
{
	if (renderThread.joinable())
	{
		renderThread.join();
	}
}

bool Gear::Core::RenderThread::waitInitialized()
{
	std::unique_lock<std::mutex> lock(taskMutex);

	taskCondition.wait(lock, [this]() {return initialized; });

	return errorOccured;
}

void Gear::Core::RenderThread::workerLoop()
{
	//申请每个渲染线程独享的描述符堆
	LocalDescriptorHeap::Internal::initialize();

#ifdef _DEBUG
	try
	{
#endif // _DEBUG
		//开始创建RenderTask
		{
			std::lock_guard<std::mutex> lockGuard(taskMutex);

			createFunc(&renderTask);

			renderTask->renderThread = this;

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

	//释放申请的独享描述符堆
	LocalDescriptorHeap::Internal::release();
}
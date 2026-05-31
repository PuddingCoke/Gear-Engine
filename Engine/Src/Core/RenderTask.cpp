#include<Gear/Core/RenderThread.h>

#include<Gear/Core/GlobalEffect/BackBufferBlitEffect.h>

Gear::Core::RenderTask::RenderTask() :
	resManager(new ResourceManager()),
	context(resManager->getGraphicsContext()),
	taskCompleted(true),
	errorOccur(false),
	isRunning(true),
	renderThread(nullptr)
{
	context->begin();
}

Gear::Core::RenderTask::~RenderTask()
{
	isRunning = false;

	beginTask();

	if (renderThread)
	{
		delete renderThread;
	}

	if (resManager)
	{
		delete resManager;
	}
}

void Gear::Core::RenderTask::beginTask()
{
	{
		std::lock_guard<std::mutex> lockGuard(taskMutex);

		taskCompleted = false;
	}

	taskCondition.notify_one();
}

bool Gear::Core::RenderTask::waitTask()
{
	std::unique_lock<std::mutex> lock(taskMutex);

	taskCondition.wait(lock, [this]() {return taskCompleted; });

	return errorOccur;
}

Gear::Core::D3D12Core::CommandList* Gear::Core::RenderTask::getCommandList() const
{
	return context->getCommandList();
}

void Gear::Core::RenderTask::imGUICall()
{

}

void Gear::Core::RenderTask::blit(Resource::TextureRenderView& texture) const
{
	GlobalEffect::BackBufferBlitEffect::process(context, texture);
}

void Gear::Core::RenderTask::workerLoop()
{
#ifdef _DEBUG
	try
	{
#endif // _DEBUG
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(taskMutex);

				taskCondition.wait(lock, [this]() {return !taskCompleted; });

				if (!isRunning)
				{
					break;
				}

				resManager->cleanTransientResources();

				context->begin();

				recordCommand();

				taskCompleted = true;
			}

			taskCondition.notify_one();
		}
#ifdef _DEBUG
	}
	catch (const std::exception&)
	{
		{
			std::unique_lock<std::mutex> lock(taskMutex);

			taskCompleted = true;

			errorOccur = true;
		}

		taskCondition.notify_one();

		return;
	}
#endif // _DEBUG
}

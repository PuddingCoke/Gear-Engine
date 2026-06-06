#include<Gear/Core/RenderThread.h>

#include<Gear/Core/GlobalEffect/BackBufferBlitEffect.h>

namespace Gear::Core
{
	RenderTask::RenderTask() :
		resManager(makeUnique<ResourceManager>()),
		context(resManager->getGraphicsContext()),
		taskCompleted(true),
		errorOccur(false),
		isRunning(true),
		renderThread(nullptr)
	{
		context->begin();
	}

	RenderTask::~RenderTask()
	{
		isRunning = false;

		beginTask();
	}

	void RenderTask::beginTask()
	{
		{
			std::lock_guard<std::mutex> lockGuard(taskMutex);

			taskCompleted = false;
		}

		taskCondition.notify_one();
	}

	bool RenderTask::waitTask()
	{
		std::unique_lock<std::mutex> lock(taskMutex);

		taskCondition.wait(lock, [this]() {return taskCompleted; });

		return errorOccur;
	}

	D3D12Core::CommandList* RenderTask::getCommandList() const
	{
		return context->getCommandList();
	}

	void RenderTask::imGUICall()
	{

	}

	void RenderTask::blit(Resource::RenderTextureView& texture) const
	{
		GlobalEffect::BackBufferBlitEffect::process(context, texture);
	}

	void RenderTask::workerLoop()
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
}

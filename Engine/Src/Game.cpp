#include<Gear/Game.h>

namespace Gear
{
	Game::Game()
	{
	}

	Game::~Game()
	{
		while (recordQueue.size())
		{
			recordQueue.front()->waitTask();

			recordQueue.pop();
		}
	}

	void Game::imGUICall()
	{
	}

	void Game::beginRenderTask(Core::RenderTask& renderTask)
	{
		if (Core::RenderEngine::getDisplayImGuiSurface())
		{
			renderTask.imGUICall();
		}

		renderTask.beginTask();

		recordQueue.push(&renderTask);
	}

	void Game::pushCreateAsync(UniquePtr<Core::RenderThread> renderThread)
	{
		createQueue.push(std::move(renderThread));
	}

	void Game::scheduleAllTasks()
	{
		while (recordQueue.size())
		{
			if (recordQueue.front()->waitTask())
			{
				throw "error occur while command recording";
			}

			Core::RenderEngine::submitCommandList(recordQueue.front()->getCommandList());

			recordQueue.pop();
		}

		while (createQueue.size())
		{
			if (createQueue.front()->waitInitialized())
			{
				throw "error occur while resource creation";
			}

			Core::RenderThread* const renderThread = createQueue.front().get();

			renderThread->transferOwnerShip(std::move(createQueue.front()));

			createQueue.pop();
		}
	}
}

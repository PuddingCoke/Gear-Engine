#include<Gear/Game.h>

namespace Gear
{
	Game::Game()
	{
	}

	Game::~Game()
	{
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
				const std::string errorStr = recordQueue.front()->getErrorStr();

				throw std::runtime_error(errorStr);
			}

			Core::RenderEngine::submitCommandList(recordQueue.front()->getCommandList());

			recordQueue.pop();
		}

		while (createQueue.size())
		{
			if (createQueue.front()->waitTask())
			{
				const std::string errorStr = createQueue.front()->getErrorStr();

				throw std::runtime_error(errorStr);
			}

			Core::RenderThread* const renderThread = createQueue.front().get();

			renderThread->transferOwnerShip(std::move(createQueue.front()));

			createQueue.pop();
		}
	}
}

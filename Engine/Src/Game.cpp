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
		renderTask.beginTask();

		recordQueue.push(&renderTask);

		if (Core::RenderEngine::getDisplayImGuiSurface())
		{
			renderTask.imGUICall();
		}
	}

	void Game::pushCreateAsync(Core::RenderThread* const renderThread)
	{
		createQueue.push(renderThread);
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

			createQueue.pop();
		}
	}
}

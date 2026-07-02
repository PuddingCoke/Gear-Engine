#include<Gear/Game.h>

#include<Gear/Core/RenderEngine.h>

namespace Gear
{
	Game::Game()
	{
	}

	Game::~Game()
	{
	}

	void Game::beginRenderTask(RenderTask& renderTask)
	{
		if (RenderEngine::getDisplayImGuiSurface())
		{
			renderTask.imGuiCall();
		}

		renderTask.beginTask();

		recordQueue.push(&renderTask);
	}

	void Game::pushCreateAsync(UniquePtr<RenderThread> renderThread)
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

				recordQueue.pop();

				//当下就改成这个样子，以后可能还会再改改
				while (recordQueue.size())
				{
					recordQueue.front()->waitTask();

					recordQueue.pop();
				}

				throw std::runtime_error(errorStr);
			}

			RenderEngine::submitCommandList(recordQueue.front()->getCommandList());

			recordQueue.pop();
		}

		while (createQueue.size())
		{
			if (createQueue.front()->waitTask())
			{
				const std::string errorStr = createQueue.front()->getErrorStr();

				createQueue.pop();

				//当下就改成这个样子，以后可能还会再改改
				while (createQueue.size())
				{
					createQueue.front()->waitTask();

					createQueue.pop();
				}

				throw std::runtime_error(errorStr);
			}

			RenderThread* const renderThread = createQueue.front().get();

			renderThread->transferOwnerShip(std::move(createQueue.front()));

			createQueue.pop();
		}
	}
}

#include <Gear/Game.h>

#include <Gear/Core/RenderEngine.h>

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
			RenderThread* const renderThread = createQueue.front().get();

			//如果出错了，暂时先只抛出遇到的第一个异常
			if (renderThread->waitTask())
			{
				const std::string errorStr = renderThread->getErrorStr();

				//RenderThread需要在由RenderTask基类析构
				//要不然DescriptorHandle析构会使用一个已释放的指针
				if (renderThread->getRenderTask())
				{
					renderThread->transferOwnerShip(std::move(createQueue.front()));
				}

				createQueue.pop();

				//进行排空
				while (createQueue.size())
				{
					createQueue.front()->waitTask();

					//排空只管RenderTask是否成功创建，成功创建了就转移所有权
					if (createQueue.front()->getRenderTask())
					{
						createQueue.front()->transferOwnerShip(std::move(createQueue.front()));
					}

					createQueue.pop();
				}

				throw std::runtime_error(errorStr);
			}

			renderThread->transferOwnerShip(std::move(createQueue.front()));

			createQueue.pop();
		}
	}
}

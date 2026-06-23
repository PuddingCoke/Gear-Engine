#pragma once

#ifndef _GEAR_GAME_H_
#define _GEAR_GAME_H_

#include<Gear/DevEssential.h>

#include<Gear/Core/RenderThread.h>

#include<queue>

namespace Gear
{
	class Game
	{
	public:

		Game();

		virtual ~Game();

		virtual void imGuiCall();

		Game(const Game&) = delete;

		void operator=(const Game&) = delete;

		virtual void update(const float dt) = 0;

		virtual void render() = 0;

		void beginRenderTask(RenderTask& renderTask);

		void pushCreateAsync(UniquePtr<RenderThread> renderThread);

		void scheduleAllTasks();

	private:

		std::queue<RenderTask*> recordQueue;

		std::queue<UniquePtr<RenderThread>> createQueue;

	};
}

#endif // !_GEAR_GAME_H_
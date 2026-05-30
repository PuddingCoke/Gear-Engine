#pragma once

#include<Gear/Game.h>

#include"MyRenderTask.h"

using namespace Gear;

class MyGame :public Game
{
public:

	MyGame()
	{
		pushCreateAsync(createRenderTaskAsync(&renderTask));

		scheduleAllTasks();
	}

	~MyGame()
	{
		delete renderTask;
	}

	void update(const float dt) override
	{

	}

	void render()
	{
		beginRenderTask(renderTask);

		scheduleAllTasks();
	}

	MyRenderTask* renderTask;
	
};
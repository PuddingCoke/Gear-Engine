#pragma once

#include<Gear/Game.h>

#include"MyRenderTask.h"

class MyGame :public Game
{
public:

	MyGame()
	{
		RenderEngine::toggleEngineImGuiSurface();

		pushCreateAsync(createRenderTaskAsync(renderTask));

		scheduleAllTasks();
	}

	~MyGame()
	{
	}

	void update(const float dt) override
	{

	}

	void render()
	{
		beginRenderTask(*renderTask);

		scheduleAllTasks();
	}

	UniquePtr<MyRenderTask> renderTask;
	
};
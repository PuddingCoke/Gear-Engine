#pragma once

#include<Gear/Game.h>

#include"MyRenderTask.h"

class MyGame :public Game
{
public:

	MyGame()
	{
		pushCreateAsync(createRenderTaskAsync(renderTask));

		scheduleAllTasks();
	}

	~MyGame()
	{
	}

	void update(const float dt) override
	{

	}

	void render() override
	{
		beginRenderTask(*renderTask);

		scheduleAllTasks();
	}

private:

	UniquePtr<MyRenderTask> renderTask;

};
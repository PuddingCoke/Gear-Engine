#pragma once

#include<Gear/Game.h>

#include"MyRenderTask.h"

class MyGame :public Game
{
public:

	MyGame():
		camera({ 4,4,-11 }, { -1,-1,-2 }, 2.f)
	{
		MainCamera::setProj(Math::pi / 4.f, Graphics::getAspectRatio(), 0.01f, 512.f);

		pushCreateAsync(createRenderTaskAsync(renderTask));

		scheduleAllTasks();
	}

	~MyGame()
	{
	}

	void update(const float dt) override
	{
		camera.rotateX(dt);

		camera.applyInput(dt);
	}

	void render()
	{
		beginRenderTask(*renderTask);

		scheduleAllTasks();
	}

	UniquePtr<MyRenderTask> renderTask;
	
	OrbitCamera camera;

};
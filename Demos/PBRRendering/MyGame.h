#pragma once

#include<Gear/Game.h>

#include"MyRenderTask.h"

class MyGame :public Game
{
public:

	MyGame() :
		camera({ 2,0,0 }, { 0,1,0 }, 1.f)
	{
		pushCreateAsync(createRenderTaskAsync(renderTask));

		scheduleAllTasks();

		MainCamera::setProj(Math::pi / 4.f, Graphics::getAspectRatio(), 0.1f, 256.f);
	}

	~MyGame()
	{
	}

	void update(const float dt) override
	{
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
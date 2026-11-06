#pragma once

#include <Gear/Game.h>

#include "MyRenderTask.h"

class MyGame : public Game
{
public:

	MyGame() : camera({ 0.f, 1.134f, 2.778f }, { 0.f, 0.f, 1.f }, 1.f, false)
	{
		pushCreateAsync(createRenderTaskAsync(renderTask));

		scheduleAllTasks();

		MainCamera::setProj(0.78539816339744f, Graphics::getAspectRatio(), 128.f, 512.f);
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

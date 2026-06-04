#pragma once

#include<Gear/Camera/FPSCamera.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Game.h>

#include"MyRenderTask.h"

using namespace Gear;

class MyGame :public Game
{
public:

	FPSCamera camera;

	MyGame():
		camera({ 0.f,20.f,0.f }, { 1.0f,0.f,0.f }, { 0.f,1.f,0.f }, 70.f)
	{
		MainCamera::setProj(Utils::Math::pi / 4.f, Graphics::getAspectRatio(), 1.f, 512.f);

		pushCreateAsync(createRenderTaskAsync(renderTask));

		scheduleAllTasks();
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
	
};
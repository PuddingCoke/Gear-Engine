#pragma once

#include<Gear/Game.h>

#include"SceneRenderTask.h"

#include"PostProcessTask.h"

class MyGame :public Game
{
public:

	MyGame() :
		camera({ 0.f,100.f,0.f }, { 1.f * sinf(0.63f),-0.1f,1.f * cosf(0.63f) }, { 0.f,1.f,0.f }, 100.f),
		originTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true, DirectX::Colors::Black))
	{
		MainCamera::setProj(Math::pi / 3.f, Graphics::getAspectRatio(), 1.f, 5000.f);

		Graphics::setExposure(0.59f);

		Graphics::setGamma(0.972f);

		originTexture->getTexture()->setName(L"originTexture");

		pushCreateAsync(createRenderTaskAsync(sceneRenderTask, &camera, *originTexture));

		pushCreateAsync(createRenderTaskAsync(postProcessTask, *originTexture));

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
		beginRenderTask(*sceneRenderTask);

		beginRenderTask(*postProcessTask);

		scheduleAllTasks();
	}

	FPSCamera camera;

	RenderTextureViewPtr originTexture;

	UniquePtr<SceneRenderTask> sceneRenderTask;

	UniquePtr<PostProcessTask> postProcessTask;

};
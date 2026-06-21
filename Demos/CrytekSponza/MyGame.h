#pragma once

#include<Gear/Game.h>

#include"SceneRenderTask.h"

#include"PostProcessTask.h"

class MyGame :public Game
{
public:

	FPSCamera camera;

	MyGame() :
		ssrCombinedTexture(ResourceManager::createGraphicsTexture(Graphics::getWidth(), Graphics::getHeight(),
			FMT::RGBA16F, 1, 1, false, true, DirectX::Colors::Black)),
		camera({ 0.f,20.f,0.f }, { 1.0f,0.f,0.f }, { 0.f,1.f,0.f }, 70.f)
	{
		MainCamera::setProj(Math::pi / 4.f, Graphics::getAspectRatio(), 1.f, 512.f);

		Graphics::setExposure(0.6f);

		Graphics::setGamma(2.2f);

		ssrCombinedTexture->getTexture()->setName(L"SSR Combined Texture");

		pushCreateAsync(createRenderTaskAsync(sceneRenderTask, *ssrCombinedTexture));

		pushCreateAsync(createRenderTaskAsync(postProcessTask, *ssrCombinedTexture));

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

	RenderTextureViewPtr ssrCombinedTexture;

	UniquePtr<SceneRenderTask> sceneRenderTask;

	UniquePtr<PostProcessTask> postProcessTask;

};
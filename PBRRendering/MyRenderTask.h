#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

#include"Scene.h"

class MyRenderTask :public RenderTask
{
public:

	Scene scene;

	TextureRenderView* renderTexture;

	TextureDepthView* depthTexture;

	MyRenderTask():
		scene("E:/Assets/PBRRendering/DNA.obj",resManager,FMT::RGBA16F)
	{
		renderTexture = ResourceManager::createTextureRenderView(Graphics::getWidth(), Graphics::getHeight(), FMT::RGBA16F, 1, 1, false, true,
			FMT::RGBA16F, FMT::UNKNOWN, FMT::RGBA16F, DirectX::Colors::Black);

		depthTexture = ResourceManager::createTextureDepthView(Graphics::getWidth(), Graphics::getHeight(), FMT::D32F, 1, 1, false, true);
	}

	~MyRenderTask()
	{
		if (renderTexture)
		{
			delete renderTexture;
		}

		if (depthTexture)
		{
			delete depthTexture;
		}
	}

protected:

	void recordCommand() override
	{
		context->setRenderTargets({ renderTexture->getRTVMipHandle(0) }, depthTexture->getDSVMipHandle(0));

		context->setViewportSimple(Graphics::getWidth(), Graphics::getHeight());

		context->transitionResources();

		context->clearRenderTarget(renderTexture->getRTVMipHandle(0), DirectX::Colors::Black);

		context->clearDepthStencil(depthTexture->getDSVMipHandle(0), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

		scene.draw(context);

		auto toneMappedTexture = ToneMapEffect::process(context, renderTexture);

		auto gammaCorrectedTexture = GammaCorrectEffect::process(context, toneMappedTexture);

		blit(gammaCorrectedTexture);
	}

};
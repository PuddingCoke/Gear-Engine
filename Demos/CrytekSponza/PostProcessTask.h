#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

#include"Scene.h"

class PostProcessTask :public RenderTask
{
public:

	PostProcessTask(const RenderTextureView& ssrCombinedTexture) :
		ssrCombinedTexture(makeUnique<RenderTextureView>(ssrCombinedTexture))
	{
		bloomEffect = BloomEffect::create(*context, Graphics::getWidth(), Graphics::getHeight(), *resManager);

		bloomEffect->setThreshold(0.f);

		bloomEffect->setSoftThreshold(0.f);

		bloomEffect->setIntensity(0.17f);

		fxaaEffect = FXAAEffect::create(*context, Graphics::getWidth(), Graphics::getHeight());
	}

	~PostProcessTask()
	{
	}

	void imGuiCall() override
	{
		bloomEffect->imGuiCall();

		fxaaEffect->imGuiCall();
	}

protected:

	void recordCommand() override
	{
		RenderTextureView* const bloomTexture = bloomEffect->process(*ssrCombinedTexture);

		RenderTextureView* const toneMappedTexture = ToneMapEffect::process(*context, *bloomTexture);

		RenderTextureView* const fxaaTexture = fxaaEffect->process(*toneMappedTexture);

		RenderTextureView* const gammaCorrectedTexture = GammaCorrectEffect::process(*context, *fxaaTexture);

		blit(*gammaCorrectedTexture);
	}

	RenderTextureViewPtr ssrCombinedTexture;

	BloomEffectPtr bloomEffect;

	FXAAEffectPtr fxaaEffect;

};
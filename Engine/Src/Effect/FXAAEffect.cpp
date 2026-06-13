#include<Gear/Effect/FXAAEffect.h>

#include<Gear/CompiledShaders/ColorToColorLuma.h>

#include<Gear/CompiledShaders/FXAA.h>

namespace Gear::Effect
{
	FXAAEffectPtr FXAAEffect::create(GraphicsContext& contextRef, const uint32_t width, const uint32_t height)
	{
		return makeUnique<FXAAEffect>(contextRef, width, height);
	}

	FXAAEffect::FXAAEffect(GraphicsContext& contextRef, const uint32_t width, const uint32_t height) :
		EffectBase(contextRef, width, height, FMT::RGBA16UN), fxaaParam{ 1.0f,0.75f,0.166f,0.0633f },
		colorLumaTexture(ResourceManager::createGraphicsTexture(width, height, FMT::RGBA16UN, 1, 1, false, true))
	{
		colorToColorLumaPS = Shader::create(g_ColorToColorLumaBytes, sizeof(g_ColorToColorLumaBytes));

		fxaaPS = Shader::create(g_FXAABytes, sizeof(g_FXAABytes));

		colorToColorLumaState = PipelineStateBuilder().setDefaultFullScreenState().setPS(*colorToColorLumaPS).setRTVFormats({ FMT::RGBA16UN }).build();

		fxaaState = PipelineStateBuilder().setDefaultFullScreenState().setPS(*fxaaPS).setRTVFormats({ FMT::RGBA16UN }).build();

		outputTexture->getTexture()->setName(L"FXAA Processed Texture");
	}

	FXAAEffect::~FXAAEffect()
	{
	}

	RenderTextureView* FXAAEffect::process(RenderTextureView& inputTexture) const
	{
		context->setPipelineState(*colorToColorLumaState);

		context->setViewportSimple(width, height);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ colorLumaTexture->getRTVMipHandle(0) });

		SETCONSTS({
		context->setPSConstants({ inputTexture.getAllSRVIndex() }, co);
			});

		context->drawQuad();

		context->setPipelineState(*fxaaState);

		context->setViewportSimple(width, height);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ outputTexture->getRTVMipHandle(0) });

		SETCONSTS({
		context->setPSConstants({ colorLumaTexture->getAllSRVIndex() }, co);

		context->setPSConstants(fxaaParam, co);
			});

		context->drawQuad();

		return outputTexture.get();
	}

	void FXAAEffect::imGUICall()
	{
		ImGui::Begin("FXAA Effect");
		ImGui::SliderFloat("FXAAQualitySubpix", &fxaaParam.fxaaQualitySubpix, 0.0f, 1.f);
		ImGui::SliderFloat("FXAAQualityEdgeThreshold", &fxaaParam.fxaaQualityEdgeThreshold, 0.0f, 1.f);
		ImGui::SliderFloat("FXAAQualityEdgeThresholdMin", &fxaaParam.fxaaQualityEdgeThresholdMin, 0.0f, 1.f);
		ImGui::End();
	}

	void FXAAEffect::setFXAAQualitySubpix(const float fxaaQualitySubpix)
	{
		fxaaParam.fxaaQualitySubpix = fxaaQualitySubpix;
	}

	void FXAAEffect::setFXAAQualityEdgeThreshold(const float fxaaQualityEdgeThreshold)
	{
		fxaaParam.fxaaQualityEdgeThreshold = fxaaQualityEdgeThreshold;
	}

	void FXAAEffect::setFXAAQualityEdgeThresholdMin(const float fxaaQualityEdgeThresholdMin)
	{
		fxaaParam.fxaaQualityEdgeThresholdMin = fxaaQualityEdgeThresholdMin;
	}
}
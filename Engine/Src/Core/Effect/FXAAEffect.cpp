#include<Gear/Core/Effect/FXAAEffect.h>

#include<Gear/CompiledShaders/ColorToColorLuma.h>

#include<Gear/CompiledShaders/FXAA.h>

namespace Gear::Core::Effect
{
	UniquePtr<FXAAEffect> FXAAEffect::create(GraphicsContext* const context, const uint32_t width, const uint32_t height)
	{
		return makeUnique<FXAAEffect>(context, width, height);
	}

	FXAAEffect::FXAAEffect(GraphicsContext* const context, const uint32_t width, const uint32_t height) :
		EffectBase(context, width, height, FMT::RGBA16UN), fxaaParam{ 1.0f,0.75f,0.166f,0.0633f },
		colorLumaTexture(ResourceManager::createTextureRenderView(width, height, FMT::RGBA16UN, 1, 1, false, true,
			FMT::RGBA16UN, FMT::UNKNOWN, FMT::RGBA16UN))
	{
		colorToColorLumaPS = D3D12Core::Shader::create(g_ColorToColorLumaBytes, sizeof(g_ColorToColorLumaBytes));

		fxaaPS = D3D12Core::Shader::create(g_FXAABytes, sizeof(g_FXAABytes));

		colorToColorLumaState = PipelineStateBuilder().setDefaultFullScreenState().setPS(*colorToColorLumaPS).setRTVFormats({ FMT::RGBA16UN }).build();

		fxaaState = PipelineStateBuilder().setDefaultFullScreenState().setPS(*fxaaPS).setRTVFormats({ FMT::RGBA16UN }).build();

		outputTexture->getTexture()->setName(L"FXAA Processed Texture");
	}

	FXAAEffect::~FXAAEffect()
	{
	}

	Resource::TextureRenderView* FXAAEffect::process(Resource::TextureRenderView& inputTexture) const
	{
		context->setPipelineState(*colorToColorLumaState);

		context->setViewportSimple(width, height);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ colorLumaTexture->getRTVMipHandle(0) });

		context->setPSConstants({ inputTexture.getAllSRVIndex() }, 0);

		context->draw(3, 1, 0, 0);

		context->setPipelineState(*fxaaState);

		context->setViewportSimple(width, height);

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ outputTexture->getRTVMipHandle(0) });

		context->setPSConstants({ colorLumaTexture->getAllSRVIndex() }, 0);

		context->setPSConstants(4, &fxaaParam, 1);

		context->draw(3, 1, 0, 0);

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
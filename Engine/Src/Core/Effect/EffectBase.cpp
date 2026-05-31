#include<Gear/Core/Effect/EffectBase.h>

Gear::Core::Effect::EffectBase::EffectBase(GraphicsContext* const context, const uint32_t width, const uint32_t height, const DXGI_FORMAT format) :
	outputTexture(ResourceManager::createTextureRenderView(width, height, format, 1, 1, false, true, format, FMT::UNKNOWN, format)),
	context(context),
	width(width),
	height(height)
{
}

Gear::Core::Effect::EffectBase::EffectBase(GraphicsContext* const context, const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat) :
	outputTexture(ResourceManager::createTextureRenderView(width, height, resFormat, arraySize, mipLevels, isTextureCube, true, srvFormat, uavFormat, rtvFormat)),
	context(context),
	width(width),
	height(height)
{
}

Gear::Core::Effect::EffectBase::~EffectBase()
{
}

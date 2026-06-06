#include<Gear/Core/Effect/EffectBase.h>

namespace Gear::Core::Effect
{
	EffectBase::EffectBase(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, const DXGI_FORMAT format) :
		outputTexture(ResourceManager::createGraphicsTexture(width, height, format, 1, 1, false, true)),
		context(&contextRef),
		width(width),
		height(height)
	{
	}

	EffectBase::EffectBase(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat) :
		outputTexture(ResourceManager::createRenderTextureView(width, height, resFormat, arraySize, mipLevels, isTextureCube, true, srvFormat, uavFormat, rtvFormat)),
		context(&contextRef),
		width(width),
		height(height)
	{
	}

	EffectBase::~EffectBase()
	{
	}
}
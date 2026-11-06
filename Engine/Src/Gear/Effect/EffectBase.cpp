#include <Gear/Effect/EffectBase.h>

namespace Gear::Effect
{
	EffectBase::EffectBase(ResourceManager& refResManager,
	                       const uint32_t width,
	                       const uint32_t height,
	                       const DXGI_FORMAT rtvFormat) :
		outputTexture(ResourceManager::createGraphicsTexture(width, height, rtvFormat, 1, 1, false, true)),
		resManager(&refResManager),
		context(refResManager.getGraphicsContext()),
		width(width),
		height(height)
	{
	}

	EffectBase::EffectBase(ResourceManager& refResManager,
	                       const uint32_t width,
	                       const uint32_t height,
	                       const DXGI_FORMAT resFormat,
	                       const uint32_t arraySize,
	                       const uint32_t mipLevels,
	                       const bool isTextureCube,
	                       const DXGI_FORMAT srvFormat,
	                       const DXGI_FORMAT uavFormat,
	                       const DXGI_FORMAT rtvFormat) :
		outputTexture(ResourceManager::createRenderTextureView(width,
	                                                           height,
	                                                           resFormat,
	                                                           arraySize,
	                                                           mipLevels,
	                                                           isTextureCube,
	                                                           true,
	                                                           srvFormat,
	                                                           uavFormat,
	                                                           rtvFormat)),
		resManager(&refResManager),
		context(refResManager.getGraphicsContext()),
		width(width),
		height(height)
	{
	}

	EffectBase::~EffectBase()
	{
	}
}

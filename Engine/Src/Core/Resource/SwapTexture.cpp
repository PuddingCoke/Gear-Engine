#include<Gear/Core/Resource/SwapTexture.h>

namespace Gear::Core::Resource
{
	SwapTexture::SwapTexture(const std::function<UniquePtr<TextureRenderView>(void)>& readTextureFunc, const std::function<UniquePtr<TextureRenderView>(void)>& writeTextureFunc) :
		SwappableBase(readTextureFunc, writeTextureFunc),
		width(read()->getTexture()->getWidth()),
		height(read()->getTexture()->getHeight())
	{
	}

	SwapTexture::~SwapTexture()
	{
	}
}
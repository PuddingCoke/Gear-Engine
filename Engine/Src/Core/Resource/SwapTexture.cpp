#include<Gear/Core/Resource/SwapTexture.h>

Gear::Core::Resource::SwapTexture::SwapTexture(const std::function<TextureRenderView* (void)>& readTextureFunc, const std::function<TextureRenderView* (void)>& writeTextureFunc) :
	SwappableBase(readTextureFunc, writeTextureFunc),
	width(read()->getTexture()->getWidth()),
	height(read()->getTexture()->getHeight())
{
}

Gear::Core::Resource::SwapTexture::SwapTexture(const std::function<TextureRenderView* (void)>& textureFunc) :
	SwapTexture(textureFunc, textureFunc)
{
}

Gear::Core::Resource::SwapTexture::~SwapTexture()
{
}

#include<Gear/Core/Resource/SwapTexture.h>

namespace Gear::Core::Resource
{
	SwapTexture::SwapTexture(const std::function<RenderTextureViewPtr(void)>& readTextureFunc, const std::function<RenderTextureViewPtr(void)>& writeTextureFunc) :
		SwappableBase(readTextureFunc, writeTextureFunc),
		width(read()->getTexture()->getWidth()),
		height(read()->getTexture()->getHeight())
	{
	}

	SwapTexture::~SwapTexture()
	{
	}
}
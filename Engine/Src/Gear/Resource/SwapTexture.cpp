#include<Gear/Resource/SwapTexture.h>

namespace Gear::Resource
{
	SwapTexture::SwapTexture(const std::function<RenderTextureViewPtr(void)>& readTextureFunc, const std::function<RenderTextureViewPtr(void)>& writeTextureFunc) :
		SwappableBase(readTextureFunc, writeTextureFunc),
		width(read()->getTexture()->getWidth()),
		height(read()->getTexture()->getHeight())
	{
	}

	RenderTargetDesc SwapTexture::getRTVMip(const uint32_t mipSlice) const
	{
		return write()->getRTVMip(mipSlice);
	}

	ShaderResourceDesc SwapTexture::getUAVMipIndex(const uint32_t mipSlice) const
	{
		return write()->getUAVMipIndex(mipSlice);
	}

	ShaderResourceDesc SwapTexture::getAllSRVIndex() const
	{
		return read()->getAllSRVIndex();
	}

	ShaderResourceDesc SwapTexture::getSRVMipIndex(const uint32_t mipSlice) const
	{
		return read()->getSRVMipIndex(mipSlice);
	}

	UAVClearDesc SwapTexture::getUAVMipClearDesc(const uint32_t mipSlice) const
	{
		return write()->getUAVMipClearDesc(mipSlice);
	}

	DirectX::XMUINT2 SwapTexture::get2Dimension(const uint32_t mipSlice) const
	{
		return write()->get2Dimension(mipSlice);
	}

	DirectX::XMUINT3 SwapTexture::get3Dimension(const uint32_t mipSlice) const
	{
		return write()->get3Dimension(mipSlice);
	}

	void SwapTexture::setName(const std::wstring& name) const
	{
		read()->getTexture()->setName((name + L" (0)").c_str());

		write()->getTexture()->setName((name + L" (1)").c_str());
	}

	SwapTexture::~SwapTexture()
	{
	}
}
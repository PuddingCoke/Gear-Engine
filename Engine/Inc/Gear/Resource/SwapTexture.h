#pragma once

#ifndef _GEAR_RESOURCE_SWAPTEXTURE_H_
#define _GEAR_RESOURCE_SWAPTEXTURE_H_

#include"RenderTextureView.h"

#include"SwappableBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(SwapTexture);

	class SwapTexture :public SwappableBase<RenderTextureView>
	{
	public:

		SwapTexture() = delete;

		SwapTexture(const std::function<RenderTextureViewPtr(void)>& readTextureFunc, const std::function<RenderTextureViewPtr(void)>& writeTextureFunc);

		RenderTargetDesc getRTVMip(const uint32_t mipSlice) const;

		ShaderResourceDesc getUAVMipIndex(const uint32_t mipSlice) const;

		ShaderResourceDesc getAllSRVIndex() const;

		ShaderResourceDesc getSRVMipIndex(const uint32_t mipSlice) const;

		UAVClearDesc getUAVMipClearDesc(const uint32_t mipSlice) const;

		DirectX::XMUINT2 get2Dimension(const uint32_t mipSlice = 0u) const;

		DirectX::XMUINT3 get3Dimension(const uint32_t mipSlice = 0u) const;

		void setName(const std::wstring& name) const;

		~SwapTexture();

		const uint32_t width;

		const uint32_t height;

	};
}

#endif // !_GEAR_RESOURCE_SWAPTEXTURE_H_
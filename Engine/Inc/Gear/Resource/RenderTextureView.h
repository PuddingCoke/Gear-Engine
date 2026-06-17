#pragma once

#ifndef _GEAR_CORE_RESOURCE_RENDERTEXTUREVIEW_H_
#define _GEAR_CORE_RESOURCE_RENDERTEXTUREVIEW_H_

#include<Gear/Core/D3D12Resource/Texture.h>

#include"ResourceBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(RenderTextureView);

	//多用途纹理
	//texturecubearray
	//texturecube
	//texture2darray
	//texture2d
	//SRV/RTV/UAV
	class RenderTextureView :public ResourceBase
	{
	public:

		RenderTextureView() = delete;

		RenderTextureView(D3D12Resource::TexturePtr texturePtr, const bool isTextureCube, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat);

		RenderTextureView(const RenderTextureView&);

		~RenderTextureView();

		D3D12Resource::ShaderResourceDesc getAllSRVIndex() const;

		D3D12Resource::ShaderResourceDesc getSRVMipIndex(const uint32_t mipSlice) const;

		D3D12_GPU_DESCRIPTOR_HANDLE getSRVMipGPUHandle(const uint32_t mipSlice) const;

		D3D12Resource::ShaderResourceDesc getUAVMipIndex(const uint32_t mipSlice) const;

		D3D12Resource::RenderTargetDesc getRTVMipHandle(const uint32_t mipSlice) const;

		D3D12Resource::ClearUAVDesc getClearUAVMipDesc(const uint32_t mipSlice) const;

		D3D12Resource::Texture* getTexture() const;

		void copyDescriptors() override;

		DXGI_FORMAT getRTVFormat() const;

		DXGI_FORMAT getUAVFormat() const;

		DirectX::XMUINT3 get3Dimension(const uint32_t mipSlice = 0u) const;

		DirectX::XMUINT2 get2Dimension(const uint32_t mipSlice = 0u) const;

	private:

		const DXGI_FORMAT rtvFormat;

		const DXGI_FORMAT uavFormat;

		SharedPtr<uint32_t> allSRVIndex;

		SharedPtr<std::vector<uint32_t>> srvMipIndices;

		SharedPtr<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> srvMipGPUHandles;

		SharedPtr<std::vector<uint32_t>> uavMipIndices;

		SharedPtr<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> rtvMipHandles;

		SharedPtr<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> viewGPUHandles;

		SharedPtr<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> viewCPUHandles;

		D3D12Resource::TexturePtr texture;
	};
}

#endif // !_GEAR_CORE_RESOURCE_RENDERTEXTUREVIEW_H_
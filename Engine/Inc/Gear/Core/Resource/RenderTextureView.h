#pragma once

#ifndef _GEAR_CORE_RESOURCE_RENDERTEXTUREVIEW_H_
#define _GEAR_CORE_RESOURCE_RENDERTEXTUREVIEW_H_

#include"D3D12Resource/Texture.h"

#include"ResourceBase.h"

namespace Gear::Core::Resource
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

		const bool hasRTV;

		const bool hasUAV;

	private:

		uint32_t allSRVIndex;

		std::vector<uint32_t> srvMipIndices;

		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> srvMipGPUHandles;

		std::vector<uint32_t> uavMipIndices;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvMipHandles;

		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> viewGPUHandles;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> viewCPUHandles;

		D3D12Resource::TexturePtr texture;
	};
}

#endif // !_GEAR_CORE_RESOURCE_RENDERTEXTUREVIEW_H_
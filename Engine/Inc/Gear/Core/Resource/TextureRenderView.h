#pragma once

#ifndef _GEAR_CORE_RESOURCE_TEXTURERENDERVIEW_H_
#define _GEAR_CORE_RESOURCE_TEXTURERENDERVIEW_H_

#include"D3D12Resource/Texture.h"

#include"ResourceBase.h"

namespace Gear::Core::Resource
{
	//多用途纹理
	//texturecubearray
	//texturecube
	//texture2darray
	//texture2d
	//SRV/RTV/UAV
	class TextureRenderView :public ResourceBase
	{
	public:

		TextureRenderView() = delete;

		TextureRenderView(D3D12Resource::Texture* const texture, const bool isTextureCube, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat);

		TextureRenderView(const TextureRenderView&);

		~TextureRenderView();

		D3D12Resource::ShaderResourceDesc getAllSRVIndex() const;

		D3D12Resource::ShaderResourceDesc getSRVMipIndex(const uint32_t mipSlice) const;

		D3D12_GPU_DESCRIPTOR_HANDLE getSRVMipGPUHandle(const uint32_t mipSlice) const;

		D3D12Resource::ShaderResourceDesc getUAVMipIndex(const uint32_t mipSlice) const;

		D3D12Resource::RenderTargetDesc getRTVMipHandle(const uint32_t mipSlice) const;

		D3D12Resource::ClearUAVDesc getClearUAVDesc(const uint32_t mipSlice) const;

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

		UniquePtr<D3D12Resource::Texture> texture;
	};
}

#endif // !_GEAR_CORE_RESOURCE_TEXTURERENDERVIEW_H_
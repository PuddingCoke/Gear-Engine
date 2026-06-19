#pragma once

#ifndef _GEAR_RESOURCE_DEPTHTEXTUREVIEW_H_
#define _GEAR_RESOURCE_DEPTHTEXTUREVIEW_H_

#include<Gear/Core/D3D12Resource/Texture.h>

#include"ResourceBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(DepthTextureView);

	class DepthTextureView :public ResourceBase
	{
	public:

		DepthTextureView() = delete;

		//支持的类型
		//R32_TYPELESS 
		//R16_TYPELESS 
		//R32G8X24_TYPELESS 
		//R24G8_TYPELESS
		DepthTextureView(D3D12Resource::TexturePtr texturePtr, const bool isTextureCube, const bool persistent);

		DepthTextureView(const DepthTextureView&);

		~DepthTextureView();

		ShaderResourceDesc getAllDepthIndex() const;

		ShaderResourceDesc getAllStencilIndex() const;

		ShaderResourceDesc getDepthMipIndex(const uint32_t mipSlice) const;

		D3D12_GPU_DESCRIPTOR_HANDLE getDepthMipGPUHandle(const uint32_t mipSlice) const;

		ShaderResourceDesc getStencilMipIndex(const uint32_t mipSlice) const;

		DepthStencilDesc getDSVMipHandle(const uint32_t mipSlice) const;

		D3D12Resource::Texture* getTexture() const;

		void copyDescriptors() override;

		DXGI_FORMAT getDSVFormat() const;

	private:

		DXGI_FORMAT depthSRVFormat;

		DXGI_FORMAT stencilSRVFormat;

		DXGI_FORMAT dsvFormat;

		SharedPtr<uint32_t> allDepthSRVIndex;

		SharedPtr<uint32_t> allStencilSRVIndex;

		SharedPtr<std::vector<uint32_t>> depthSRVMipIndices;

		SharedPtr<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> depthSRVMipGPUHandles;

		SharedPtr<std::vector<uint32_t>> stencilSRVMipIndices;

		SharedPtr<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> dsvMipHandles;

		D3D12Resource::TexturePtr texture;
	};
}

#endif // !_GEAR_RESOURCE_DEPTHTEXTUREVIEW_H_
#pragma once

#ifndef _GEAR_CORE_RESOURCE_DEPTHTEXTUREVIEW_H_
#define _GEAR_CORE_RESOURCE_DEPTHTEXTUREVIEW_H_

#include"D3D12Resource/Texture.h"

#include"ResourceBase.h"

namespace Gear::Core::Resource
{
	class DepthTextureView :public ResourceBase
	{
	public:

		DepthTextureView() = delete;

		//支持的类型
		//R32_TYPELESS 
		//R16_TYPELESS 
		//R32G8X24_TYPELESS 
		//R24G8_TYPELESS
		DepthTextureView(UniquePtr<D3D12Resource::Texture> texturePtr, const bool isTextureCube, const bool persistent);

		DepthTextureView(const DepthTextureView&);

		~DepthTextureView();

		D3D12Resource::ShaderResourceDesc getAllDepthIndex() const;

		D3D12Resource::ShaderResourceDesc getAllStencilIndex() const;

		D3D12Resource::ShaderResourceDesc getDepthMipIndex(const uint32_t mipSlice) const;

		D3D12_GPU_DESCRIPTOR_HANDLE getDepthMipGPUHandle(const uint32_t mipSlice) const;

		D3D12Resource::ShaderResourceDesc getStencilMipIndex(const uint32_t mipSlice) const;

		D3D12Resource::DepthStencilDesc getDSVMipHandle(const uint32_t mipSlice) const;

		D3D12Resource::Texture* getTexture() const;

		void copyDescriptors() override;

	private:

		bool hasDepthSRV;

		bool hasStencilSRV;

		uint32_t allDepthSRVIndex;

		uint32_t allStencilSRVIndex;

		std::vector<uint32_t> depthSRVMipIndices;

		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> depthSRVMipGPUHandles;

		std::vector<uint32_t> stencilSRVMipIndices;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> dsvMipHandles;

		UniquePtr<D3D12Resource::Texture> texture;
	};
}

#endif // !_GEAR_CORE_RESOURCE_DEPTHTEXTUREVIEW_H_
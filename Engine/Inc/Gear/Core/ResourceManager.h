#pragma once

#ifndef _GEAR_CORE_RESOURCEMANAGER_H_
#define _GEAR_CORE_RESOURCEMANAGER_H_

#include<Gear/Core/GraphicsContext.h>

#include<Gear/Resource/SwapTexture.h>

#include<Gear/Resource/SwapBuffer.h>

#include<Gear/Resource/ImmutableCBuffer.h>

#include<Gear/Resource/DefaultCBuffer.h>

#include<Gear/Resource/DynamicCbuffer.h>

#include<Gear/Resource/BufferView.h>

#include<Gear/Resource/RenderTextureView.h>

#include<Gear/Resource/DepthTextureView.h>

#include<Gear/Resource/ImmutableIndexCBuffer.h>

namespace Gear::Core
{
	enum class RandomDataType
	{
		NOISE,
		GAUSS
	};

	CREATESAFETYPE(ResourceManager);

	//用于创建低级资源和高级资源
	//使用数据创建资源会需要CommandList，因此会需要一个ResourceManager实例
	class ResourceManager
	{
	public:

		ResourceManager(const ResourceManager&) = delete;

		void operator=(const ResourceManager&) = delete;

		ResourceManager();

		~ResourceManager();

		//延迟释放低级资源
		void deferredRelease(UniquePtr<D3D12Resource::D3D12ResourceBase> d3d12Resource);

		//延迟释放高级资源
		void deferredRelease(UniquePtr<Resource::ResourceBase> resource);

		void cleanTransientResources();

		GraphicsContext* getGraphicsContext() const;

		D3D12Core::CommandList* getCommandList() const;

		//以下的方法用于创建并使用数据来初始化低级资源

		//从数据创建缓冲
		D3D12Resource::BufferPtr createBuffer(const void* const data, const uint64_t size, const D3D12_RESOURCE_FLAGS resFlags);

		//从文件创建纹理
		D3D12Resource::TexturePtr createTexture(const std::wstring& filePath, const D3D12_RESOURCE_FLAGS resFlags, bool* const isTextureCube);

		//从随机数据创建纹理
		D3D12Resource::TexturePtr createTexture(const uint32_t width, const uint32_t height, const RandomDataType type, const D3D12_RESOURCE_FLAGS resFlags);

		//以下的方法用于创建高级资源

		Resource::ImmutableCBufferPtr createImmutableCBuffer(const uint32_t size, const void* const data, const bool persistent);

		Resource::DefaultCBufferPtr createDefaultCBuffer(const uint32_t size, const void* const data, const bool persistent);

		static Resource::DefaultCBufferPtr createDefaultCBuffer(const uint32_t size, const bool persistent);

		static Resource::DynamicCBufferPtr createDynamicCBuffer(const uint32_t size, const void* const data = nullptr);

		Resource::BufferViewPtr createTypedBufferView(const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent, const void* const data);

		static Resource::BufferViewPtr createTypedBufferView(const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent);

		Resource::BufferViewPtr createStructuredBufferView(const uint32_t structureByteStride, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool cpuWritable, const bool persistent, const void* const data);

		static Resource::BufferViewPtr createStructuredBufferView(const uint32_t structureByteStride, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool cpuWritable, const bool persistent);

		Resource::BufferViewPtr createByteAddressBufferView(const uint64_t size, const bool createSRV, const bool createUAV, const bool cpuWritable, const bool persistent, const void* const data);

		static Resource::BufferViewPtr createByteAddressBufferView(const uint64_t size, const bool createSRV, const bool createUAV, const bool cpuWritable, const bool persistent);

		static Resource::DepthTextureViewPtr createDepthTextureView(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent);

		Resource::RenderTextureViewPtr createRenderTextureView(const std::wstring& filePath, const bool persistent, const bool hasUAV = false, const bool hasRTV = false);

		Resource::RenderTextureViewPtr createRenderTextureView(const uint32_t width, const uint32_t height, const RandomDataType type, const bool persistent,
			const DXGI_FORMAT srvFormat = FMT::UNKNOWN, const DXGI_FORMAT uavFormat = FMT::UNKNOWN, const DXGI_FORMAT rtvFormat = FMT::UNKNOWN);

		//创建能同时用于图形管线和计算管线或自定义的高级纹理
		static Resource::RenderTextureViewPtr createRenderTextureView(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent,
			DXGI_FORMAT srvFormat = FMT::UNKNOWN, DXGI_FORMAT uavFormat = FMT::UNKNOWN, DXGI_FORMAT rtvFormat = FMT::UNKNOWN, const float* const color = nullptr);

		//创建仅用于图形管线的高级纹理
		static Resource::RenderTextureViewPtr createGraphicsTexture(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent,
			const float* const color = nullptr, DXGI_FORMAT srvFormat = FMT::UNKNOWN, DXGI_FORMAT rtvFormat = FMT::UNKNOWN);

		//创建仅用于计算管线的高级纹理
		static Resource::RenderTextureViewPtr createComputeTexture(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent,
			DXGI_FORMAT srvFormat = FMT::UNKNOWN, DXGI_FORMAT uavFormat = FMT::UNKNOWN);

		Resource::RenderTextureViewPtr createTextureCube(const std::wstring& filePath, const uint32_t texturecubeResolution, const bool persistent, const bool hasUAV = false, const bool hasRTV = false);

		Resource::RenderTextureViewPtr createTextureCube(const std::initializer_list<std::wstring>& texturesPath, const bool persistent,
			const DXGI_FORMAT srvFormat = FMT::UNKNOWN, const DXGI_FORMAT uavFormat = FMT::UNKNOWN, const DXGI_FORMAT rtvFormat = FMT::UNKNOWN);

		static Resource::SwapBufferPtr createSwapBuffer(const std::function<Resource::BufferViewPtr(void)>& readBufferFunc, const std::function<Resource::BufferViewPtr(void)>& writeBufferFunc);

		static Resource::SwapBufferPtr createSwapBuffer(const std::function<Resource::BufferViewPtr(void)>& bufferFunc);

		static Resource::SwapTexturePtr createSwapTexture(const std::function<Resource::RenderTextureViewPtr(void)>& readTextureFunc, const std::function<Resource::RenderTextureViewPtr(void)>& writeTextureFunc);

		static Resource::SwapTexturePtr createSwapTexture(const std::function<Resource::RenderTextureViewPtr(void)>& textureFunc);

		template<size_t N>
		Resource::ImmutableIndexCBufferPtr createImmutableIndexCBuffer(const Resource::ResourceIndexPair(&pairs)[N]);

	protected:

		//处理高级任务，比如从等距柱状图创建立方体贴图
		GraphicsContextPtr context;

		//处理低级任务，比如用数据初始化纹理
		//注意：这个指针只是引用
		D3D12Core::CommandList* const commandList;

	private:

		UniquePtr<std::vector<UniquePtr<D3D12Resource::D3D12ResourceBase>>[]> d3d12Resources;

		UniquePtr<std::vector<UniquePtr<Resource::ResourceBase>>[]> resources;

	};

	template<size_t N>
	inline Resource::ImmutableIndexCBufferPtr ResourceManager::createImmutableIndexCBuffer(const Resource::ResourceIndexPair(&pairs)[N])
	{
#ifdef _DEBUG
		for (uint32_t i = 0; i < N; i++)
		{
			if (!(pairs[i].first->getPersistent()))
			{
				LOGERROR(L"侦测到传入的资源中有非持久性资源，无法为非持久性资源创建不可变索引常量缓冲");
			}
		}
#endif // _DEBUG

		const uint64_t byteSize = Resource::getCBufferByteSizeFromPairs(pairs);

		const uint64_t numElement = byteSize / sizeof(uint32_t);

		std::vector<uint32_t> resourceIndices = std::vector<uint32_t>(numElement);

		for (uint32_t i = 0; i < N; i++)
		{
			resourceIndices[i] = pairs[i].second().resourceIndex;
		}

		D3D12Resource::BufferPtr buffer = createBuffer(resourceIndices.data(), byteSize, D3D12_RESOURCE_FLAG_NONE);

		return makeUnique<Resource::ImmutableIndexCBuffer>(pairs, std::move(buffer));
	}
}

#endif // !_GEAR_CORE_RESOURCEMANAGER_H_
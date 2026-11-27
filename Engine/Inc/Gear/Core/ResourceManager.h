#pragma once

#ifndef _GEAR_CORE_RESOURCEMANAGER_H_
#define _GEAR_CORE_RESOURCEMANAGER_H_

#include <Gear/Core/GraphicsContext.h>

#include <Gear/Core/FMT.h>

#include <Gear/Resource/SwapTexture.h>

#include <Gear/Resource/SwapBuffer.h>

#include <Gear/Resource/ImmutableCBuffer.h>

#include <Gear/Resource/DefaultCBuffer.h>

#include <Gear/Resource/DynamicCbuffer.h>

#include <Gear/Resource/IndexTable.h>

#include <Gear/Resource/ImmutableIndexTable.h>

#include <Gear/Resource/DefaultIndexTable.h>

#include <Gear/Resource/DynamicIndexTable.h>

#include <Gear/Resource/BufferView.h>

#include <Gear/Resource/RenderTextureView.h>

#include <Gear/Resource/DepthTextureView.h>

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
		D3D12Resource::BufferPtr createBuffer(const void* const data,
		                                      const uint64_t size,
		                                      const D3D12_RESOURCE_FLAGS resFlags);

		//从文件创建纹理
		D3D12Resource::TexturePtr createTexture(const Utils::File::Path& filePath,
		                                        const D3D12_RESOURCE_FLAGS resFlags,
		                                        bool* const isTextureCube);

		//从随机数据创建纹理
		D3D12Resource::TexturePtr createTexture(const uint32_t width,
		                                        const uint32_t height,
		                                        const RandomDataType type,
		                                        const D3D12_RESOURCE_FLAGS resFlags);

		//以下的方法用于创建高级资源

		Resource::ImmutableCBufferPtr createImmutableCBuffer(const uint32_t size,
		                                                     const void* const data);

		Resource::DefaultCBufferPtr createDefaultCBuffer(const uint32_t size,
		                                                 const void* const data);

		static Resource::DefaultCBufferPtr createDefaultCBuffer(const uint32_t size);

		static Resource::DynamicCBufferPtr createDynamicCBuffer(const uint32_t size,
		                                                        const void* const data = nullptr);

		//创建索引表，不可变类型会在此一并完成唯一的一次上传
		template<size_t N>
		Resource::ImmutableIndexTablePtr createImmutableIndexTable(
			const Resource::ShaderResourceDesc (&descs)[N])
		{
			static_assert(N <= Resource::IndexTable::maxIndexNum);

			//缓冲区固定为byteSize字节(256)，超出资源描述数量的部分填上无效索引
			uint32_t indices[Resource::IndexTable::maxIndexNum];

			for (uint32_t i = 0; i < Resource::IndexTable::maxIndexNum; i++)
			{
				indices[i] = UINT32_MAX;
			}

			for (uint32_t i = 0; i < N; i++)
			{
				indices[i] = descs[i].resourceIndex;
			}

			Resource::ImmutableCBufferPtr cbuffer =
				createImmutableCBuffer(Resource::IndexTable::byteSize, indices);

			return makeUnique<Resource::ImmutableIndexTable>(std::move(cbuffer), descs);
		}

		template<size_t N>
		static Resource::DefaultIndexTablePtr createDefaultIndexTable(
			const Resource::ShaderResourceDesc (&descs)[N])
		{
			static_assert(N <= Resource::IndexTable::maxIndexNum);

			//缓冲区的内容由索引表在第一次使用时上传
			Resource::DefaultCBufferPtr cbuffer =
				createDefaultCBuffer(Resource::IndexTable::byteSize);

			return makeUnique<Resource::DefaultIndexTable>(std::move(cbuffer), descs);
		}

		template<size_t N>
		static Resource::DynamicIndexTablePtr createDynamicIndexTable(
			const Resource::ShaderResourceDesc (&descs)[N])
		{
			static_assert(N <= Resource::IndexTable::maxIndexNum);

			return makeUnique<Resource::DynamicIndexTable>(descs);
		}

		Resource::BufferViewPtr createTypedBufferView(const DXGI_FORMAT format,
		                                              const uint64_t size,
		                                              const bool createSRV,
		                                              const bool createUAV,
		                                              const bool createVBV,
		                                              const bool createIBV,
		                                              const bool cpuWritable,
		                                              const void* const data);

		static Resource::BufferViewPtr createTypedBufferView(const DXGI_FORMAT format,
		                                                     const uint64_t size,
		                                                     const bool createSRV,
		                                                     const bool createUAV,
		                                                     const bool createVBV,
		                                                     const bool createIBV,
		                                                     const bool cpuWritable);

		Resource::BufferViewPtr createStructuredBufferView(const uint32_t structureByteStride,
		                                                   const uint64_t size,
		                                                   const bool createSRV,
		                                                   const bool createUAV,
		                                                   const bool createVBV,
		                                                   const bool cpuWritable,
		                                                   const void* const data);

		static Resource::BufferViewPtr createStructuredBufferView(const uint32_t structureByteStride,
		                                                          const uint64_t size,
		                                                          const bool createSRV,
		                                                          const bool createUAV,
		                                                          const bool createVBV,
		                                                          const bool cpuWritable);

		Resource::BufferViewPtr createByteAddressBufferView(const uint64_t size,
		                                                    const bool createSRV,
		                                                    const bool createUAV,
		                                                    const bool cpuWritable,
		                                                    const void* const data);

		static Resource::BufferViewPtr createByteAddressBufferView(const uint64_t size,
		                                                           const bool createSRV,
		                                                           const bool createUAV,
		                                                           const bool cpuWritable);

		static Resource::DepthTextureViewPtr createDepthTextureView(const uint32_t width,
		                                                            const uint32_t height,
		                                                            const DXGI_FORMAT resFormat,
		                                                            const uint32_t arraySize,
		                                                            const uint32_t mipLevels,
		                                                            const bool isTextureCube);

		Resource::RenderTextureViewPtr createRenderTextureView(const Utils::File::Path& filePath,
		                                                       const bool hasUAV = false,
		                                                       const bool hasRTV = false);

		Resource::RenderTextureViewPtr createRenderTextureView(const uint32_t width,
		                                                       const uint32_t height,
		                                                       const RandomDataType type,
		                                                       const DXGI_FORMAT srvFormat = FMT::UNKNOWN,
		                                                       const DXGI_FORMAT uavFormat = FMT::UNKNOWN,
		                                                       const DXGI_FORMAT rtvFormat = FMT::UNKNOWN);

		//创建能同时用于图形管线和计算管线或自定义的高级纹理
		static Resource::RenderTextureViewPtr createRenderTextureView(const uint32_t width,
		                                                              const uint32_t height,
		                                                              const DXGI_FORMAT resFormat,
		                                                              const uint32_t arraySize,
		                                                              const uint32_t mipLevels,
		                                                              const bool isTextureCube,
		                                                              DXGI_FORMAT srvFormat = FMT::UNKNOWN,
		                                                              DXGI_FORMAT uavFormat = FMT::UNKNOWN,
		                                                              DXGI_FORMAT rtvFormat = FMT::UNKNOWN,
		                                                              const float* const color = nullptr);

		//创建仅用于图形管线的高级纹理
		static Resource::RenderTextureViewPtr createGraphicsTexture(const uint32_t width,
		                                                            const uint32_t height,
		                                                            const DXGI_FORMAT resFormat,
		                                                            const uint32_t arraySize,
		                                                            const uint32_t mipLevels,
		                                                            const bool isTextureCube,
		                                                            const float* const color = nullptr,
		                                                            DXGI_FORMAT srvFormat = FMT::UNKNOWN,
		                                                            DXGI_FORMAT rtvFormat = FMT::UNKNOWN);

		//创建仅用于计算管线的高级纹理
		static Resource::RenderTextureViewPtr createComputeTexture(const uint32_t width,
		                                                           const uint32_t height,
		                                                           const DXGI_FORMAT resFormat,
		                                                           const uint32_t arraySize,
		                                                           const uint32_t mipLevels,
		                                                           const bool isTextureCube,
		                                                           DXGI_FORMAT srvFormat = FMT::UNKNOWN,
		                                                           DXGI_FORMAT uavFormat = FMT::UNKNOWN);

		Resource::RenderTextureViewPtr createTextureCube(const Utils::File::Path& filePath,
		                                                 const uint32_t texturecubeResolution,
		                                                 const bool hasUAV = false,
		                                                 const bool hasRTV = false);

		Resource::RenderTextureViewPtr createTextureCube(
			const std::initializer_list<Utils::File::Path>& texturesPath,
			const DXGI_FORMAT srvFormat = FMT::UNKNOWN,
			const DXGI_FORMAT uavFormat = FMT::UNKNOWN,
			const DXGI_FORMAT rtvFormat = FMT::UNKNOWN);

		static Resource::SwapBufferPtr createSwapBuffer(
			const std::function<Resource::BufferViewPtr(void)>& readBufferFunc,
			const std::function<Resource::BufferViewPtr(void)>& writeBufferFunc);

		static Resource::SwapBufferPtr createSwapBuffer(
			const std::function<Resource::BufferViewPtr(void)>& bufferFunc);

		static Resource::SwapTexturePtr createSwapTexture(
			const std::function<Resource::RenderTextureViewPtr(void)>& readTextureFunc,
			const std::function<Resource::RenderTextureViewPtr(void)>& writeTextureFunc);

		static Resource::SwapTexturePtr createSwapTexture(
			const std::function<Resource::RenderTextureViewPtr(void)>& textureFunc);

	protected:

		GraphicsContextPtr context;

	private:

		UniquePtr<std::vector<UniquePtr<D3D12Resource::D3D12ResourceBase>>[]> d3d12Resources;

		UniquePtr<std::vector<UniquePtr<Resource::ResourceBase>>[]> resources;
	};
}

#endif // !_GEAR_CORE_RESOURCEMANAGER_H_

#pragma once

#ifndef _GEAR_CORE_RESOURCEMANAGER_H_
#define _GEAR_CORE_RESOURCEMANAGER_H_

#include<Gear/Core/GraphicsContext.h>

#include<Gear/Core/Resource/SwapTexture.h>

#include<Gear/Core/Resource/SwapBuffer.h>

#include<Gear/Core/Resource/ImmutableCBuffer.h>

#include<Gear/Core/Resource/StaticCBuffer.h>

#include<Gear/Core/Resource/DynamicCbuffer.h>

#include<Gear/Core/Resource/BufferView.h>

#include<Gear/Core/Resource/TextureRenderView.h>

#include<Gear/Core/Resource/TextureDepthView.h>

namespace Gear
{
	namespace Core
	{
		enum class RandomDataType
		{
			NOISE,
			GAUSS
		};

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
			void deferredRelease(Resource::D3D12Resource::D3D12ResourceBase* const d3d12Resource);

			//延迟释放高级资源
			void deferredRelease(Resource::ResourceBase* const resource);

			void cleanTransientResources();

			GraphicsContext* getGraphicsContext() const;

			D3D12Core::CommandList* getCommandList() const;

			//以下的方法用于创建并使用数据来初始化低级资源

			//从数据创建缓冲
			Resource::D3D12Resource::Buffer* createBuffer(const void* const data, const uint64_t size, const D3D12_RESOURCE_FLAGS resFlags);

			//从文件创建纹理
			Resource::D3D12Resource::Texture* createTexture(const std::wstring& filePath, const D3D12_RESOURCE_FLAGS resFlags, bool* const isTextureCube);

			//从随机数据创建纹理
			Resource::D3D12Resource::Texture* createTexture(const uint32_t width, const uint32_t height, const RandomDataType type, const D3D12_RESOURCE_FLAGS resFlags);

			//以下的方法用于创建高级资源

			Resource::ImmutableCBuffer* createImmutableCBuffer(const uint32_t size, const void* const data, const bool persistent);

			Resource::StaticCBuffer* createStaticCBuffer(const uint32_t size, const void* const data, const bool persistent);

			static Resource::StaticCBuffer* createStaticCBuffer(const uint32_t size, const bool persistent);

			static Resource::DynamicCBuffer* createDynamicCBuffer(const uint32_t size, const void* const data = nullptr);

			Resource::BufferView* createTypedBufferView(const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent, const void* const data);

			static Resource::BufferView* createTypedBufferView(const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent);

			Resource::BufferView* createStructuredBufferView(const uint32_t structureByteStride, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool cpuWritable, const bool persistent, const void* const data);

			static Resource::BufferView* createStructuredBufferView(const uint32_t structureByteStride, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool cpuWritable, const bool persistent);

			Resource::BufferView* createByteAddressBufferView(const uint64_t size, const bool createSRV, const bool createUAV, const bool cpuWritable, const bool persistent, const void* const data);

			static Resource::BufferView* createByteAddressBufferView(const uint64_t size, const bool createSRV, const bool createUAV, const bool cpuWritable, const bool persistent);

			static Resource::TextureDepthView* createTextureDepthView(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent);

			Resource::TextureRenderView* createTextureRenderView(const std::wstring& filePath, const bool persistent, const bool hasUAV = false, const bool hasRTV = false);

			Resource::TextureRenderView* createTextureRenderView(const uint32_t width, const uint32_t height, const RandomDataType type, const bool persistent,
				const DXGI_FORMAT srvFormat = FMT::UNKNOWN, const DXGI_FORMAT uavFormat = FMT::UNKNOWN, const DXGI_FORMAT rtvFormat = FMT::UNKNOWN);

			static Resource::TextureRenderView* createTextureRenderView(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent,
				const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat, const float* const color = nullptr);

			Resource::TextureRenderView* createTextureCube(const std::wstring& filePath, const uint32_t texturecubeResolution, const bool persistent, const bool hasUAV = false, const bool hasRTV = false);

			Resource::TextureRenderView* createTextureCube(const std::initializer_list<std::wstring>& texturesPath, const bool persistent,
				const DXGI_FORMAT srvFormat = FMT::UNKNOWN, const DXGI_FORMAT uavFormat = FMT::UNKNOWN, const DXGI_FORMAT rtvFormat = FMT::UNKNOWN);

		protected:

			//处理高级任务，比如从等距柱状图创建立方体贴图
			GraphicsContext* const context;

			//处理低级任务，比如用数据初始化纹理
			//注意：这个指针只是引用
			D3D12Core::CommandList* const commandList;

		private:

			std::vector<Resource::D3D12Resource::D3D12ResourceBase*>* d3d12Resources;

			std::vector<Resource::ResourceBase*>* resources;

		};
	}
}

#endif // !_GEAR_CORE_RESOURCEMANAGER_H_
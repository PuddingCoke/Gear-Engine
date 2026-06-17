#include<Gear/Core/ResourceManager.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Effect/HDRClampEffect.h>

#include<Gear/Effect/LatLongMapToCubeMapEffect.h>

#include<Gear/Utils/Math.h>

#include<Gear/Utils/Random.h>

#include<Gear/Utils/File.h>

#include<DirectXTex/DDSTextureLoader12.h>

#include<DirectXTex/WICTextureLoader12.h>

#include<DirectXTex/DirectXTexEXR.h>

#include<DirectXTex/DirectXTex.h>

namespace Gear::Core
{
	ResourceManager::ResourceManager() :
		context(makeUnique<GraphicsContext>()), commandList(context->getCommandList()),
		d3d12Resources(makeUnique<std::vector<UniquePtr<D3D12Resource::D3D12ResourceBase>>[]>(Graphics::getFrameBufferCount())),
		resources(makeUnique<std::vector<UniquePtr<Resource::ResourceBase>>[]>(Graphics::getFrameBufferCount()))
	{
	}

	ResourceManager::~ResourceManager()
	{
	}

	void ResourceManager::deferredRelease(UniquePtr<D3D12Resource::D3D12ResourceBase> d3d12Resource)
	{
		d3d12Resources[Graphics::getFrameIndex()].emplace_back(std::move(d3d12Resource));
	}

	void ResourceManager::deferredRelease(UniquePtr<Resource::ResourceBase> resource)
	{
		resources[Graphics::getFrameIndex()].emplace_back(std::move(resource));
	}

	void ResourceManager::cleanTransientResources()
	{
		d3d12Resources[Graphics::getFrameIndex()].clear();

		resources[Graphics::getFrameIndex()].clear();
	}

	GraphicsContext* ResourceManager::getGraphicsContext() const
	{
		return context.get();
	}

	D3D12Core::CommandList* ResourceManager::getCommandList() const
	{
		return commandList;
	}

	D3D12Resource::BufferPtr ResourceManager::createBuffer(const void* const data, const uint64_t size, const D3D12_RESOURCE_FLAGS resFlags)
	{
		D3D12Resource::BufferPtr buffer = makeUnique<D3D12Resource::Buffer>(size, true, resFlags);

		UniquePtr<D3D12Resource::UploadHeap> uploadHeap = makeUnique<D3D12Resource::UploadHeap>(size);

		uploadHeap->update(data, size);

		commandList->copyBufferRegion(buffer.get(), 0, uploadHeap.get(), 0, size);

		deferredRelease(std::move(uploadHeap));

		return buffer;
	}

	D3D12Resource::TexturePtr ResourceManager::createTexture(const std::wstring& filePath, const D3D12_RESOURCE_FLAGS resFlags, bool* const isTextureCube)
	{
		if (!Utils::File::exist(filePath))
		{
			LOGERROR(filePath, L"指定路径下未找到文件");
		}

		D3D12Resource::TexturePtr texture;

		const std::wstring fileExtension = Utils::File::getExtension(filePath);

		if (isTextureCube)
		{
			*isTextureCube = false;
		}

		//这里要小心点，因为D3D12资源是由第三方库创建的，所以要注意资源的初始状态
		//我查看了下源码发现资源一开始其实是处于D3D12_RESOURCE_STATE_COMMON状态
		//但是这里有一个叫Implicit State Transitions即隐式状态转变的东西要注意
		//因为我很久以前在调试引擎的时候发现了一个奇怪的报错现象，搜索相关资料后找到了下面这个网站
		//https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#implicit-state-transitions
		//资源一开始虽然是处于COMMOM状态，但是UpdateSubresources中存在CopyBufferRegion
		//这个操作执行完后，资源的状态实际上会从D3D12_RESOURCE_STATE_COMMON被提升到D3D12_RESOURCE_STATE_COPY_DEST
		//这就是为什么引擎内部追踪的初始状态会被设置为D3D12_RESOURCE_STATE_COPY_DEST
		if (fileExtension == L"bmp" || fileExtension == L"jpg" || fileExtension == L"jpeg" || fileExtension == L"png" || fileExtension == L"tiff")
		{
			std::unique_ptr<uint8_t[]> decodedData;

			D3D12_SUBRESOURCE_DATA subresource;

			ComPtr<ID3D12Resource> tex;

			CHECKERROR(DirectX::LoadWICTextureFromFileEx(GraphicsDevice::get(), filePath.c_str(), 0, resFlags, DirectX::WIC_LOADER_DEFAULT, &tex, decodedData, subresource));

			texture = makeUnique<D3D12Resource::Texture>(tex, true, D3D12_RESOURCE_STATE_COPY_DEST);

			const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, 1);

			UniquePtr<D3D12Resource::UploadHeap> uploadHeap = makeUnique<D3D12Resource::UploadHeap>(uploadHeapSize);

			UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, 1, &subresource);

			deferredRelease(std::move(uploadHeap));
		}
		else if (fileExtension == L"dds")
		{
			std::unique_ptr<uint8_t[]> decodedData;

			std::vector<D3D12_SUBRESOURCE_DATA> subresources;

			ComPtr<ID3D12Resource> tex;

			CHECKERROR(DirectX::LoadDDSTextureFromFileEx(GraphicsDevice::get(), filePath.c_str(), 0, resFlags, DirectX::DDS_LOADER_DEFAULT, &tex, decodedData, subresources, nullptr, isTextureCube));

			texture = makeUnique<D3D12Resource::Texture>(tex, true, D3D12_RESOURCE_STATE_COPY_DEST);

			const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, static_cast<uint32_t>(subresources.size()));

			UniquePtr<D3D12Resource::UploadHeap> uploadHeap = makeUnique<D3D12Resource::UploadHeap>(uploadHeapSize);

			UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, static_cast<uint32_t>(subresources.size()), subresources.data());

			deferredRelease(std::move(uploadHeap));
		}
		else if (fileExtension == L"hdr" || fileExtension == L"tga" || fileExtension == L"exr")
		{
			DirectX::TexMetadata metadata;

			DirectX::ScratchImage scratchImage;

			if (fileExtension == L"hdr")
			{
				CHECKERROR(DirectX::LoadFromHDRFile(filePath.c_str(), &metadata, scratchImage));
			}
			else if (fileExtension == L"tga")
			{
				CHECKERROR(DirectX::LoadFromTGAFile(filePath.c_str(), &metadata, scratchImage));
			}
			else
			{
				CHECKERROR(DirectX::LoadFromEXRFile(filePath.c_str(), &metadata, scratchImage));
			}

			const DirectX::Image* image = scratchImage.GetImage(0, 0, 0);

			D3D12_SUBRESOURCE_DATA subresource;
			subresource.pData = image->pixels;
			subresource.RowPitch = image->rowPitch;
			subresource.SlicePitch = image->slicePitch;

			ComPtr<ID3D12Resource> tex;

			CHECKERROR(DirectX::CreateTextureEx(GraphicsDevice::get(), metadata, resFlags, DirectX::CREATETEX_DEFAULT, &tex));

			texture = makeUnique<D3D12Resource::Texture>(tex, true, D3D12_RESOURCE_STATE_COPY_DEST);

			const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, 1);

			UniquePtr<D3D12Resource::UploadHeap> uploadHeap = makeUnique<D3D12Resource::UploadHeap>(uploadHeapSize);

			UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, 1, &subresource);

			deferredRelease(std::move(uploadHeap));
		}
		else
		{
			LOGERROR(fileExtension, L"是不被支持的文件扩展名！");
		}

		LOGSUCCESS(L"读取", LogColor::brightBlue, filePath);

		return texture;
	}

	D3D12Resource::TexturePtr ResourceManager::createTexture(const uint32_t width, const uint32_t height, const RandomDataType type, const D3D12_RESOURCE_FLAGS resFlags)
	{
		D3D12Resource::TexturePtr texture;

		if (type == RandomDataType::NOISE)
		{
			struct Col
			{
				uint8_t r, g, b, a;
			};

			std::vector<Col> colors(width * height);

			for (uint32_t i = 0; i < width * height; i++)
			{
				colors[i] =
				{
					static_cast<uint8_t>(Utils::Random::genUint() % 256u),
					static_cast<uint8_t>(Utils::Random::genUint() % 256u),
					static_cast<uint8_t>(Utils::Random::genUint() % 256u),
					static_cast<uint8_t>(Utils::Random::genUint() % 256u)
				};
			}

			texture = makeUnique<D3D12Resource::Texture>(width, height, FMT::RGBA8UN, 1, 1, true, resFlags);

			const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, 1);

			UniquePtr<D3D12Resource::UploadHeap> uploadHeap = makeUnique<D3D12Resource::UploadHeap>(uploadHeapSize);

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = colors.data();
			subresourceData.RowPitch = width * 4u;
			subresourceData.SlicePitch = subresourceData.RowPitch * height;

			UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, 1, &subresourceData);

			deferredRelease(std::move(uploadHeap));
		}
		else
		{
			struct HaflColor
			{
				DirectX::PackedVector::HALF r, g, b, a;
			};

			std::vector<HaflColor> colors(width * height);

			for (uint32_t i = 0; i < width * height; i++)
			{
				colors[i] =
				{
					DirectX::PackedVector::XMConvertFloatToHalf(Utils::Random::genGauss()),
					DirectX::PackedVector::XMConvertFloatToHalf(Utils::Random::genGauss()),
					DirectX::PackedVector::XMConvertFloatToHalf(Utils::Random::genGauss()),
					DirectX::PackedVector::XMConvertFloatToHalf(Utils::Random::genGauss())
				};
			}

			texture = makeUnique<D3D12Resource::Texture>(width, height, FMT::RGBA16F, 1, 1, true, resFlags);

			const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, 1);

			UniquePtr<D3D12Resource::UploadHeap> uploadHeap = makeUnique<D3D12Resource::UploadHeap>(uploadHeapSize);

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = colors.data();
			subresourceData.RowPitch = width * 8u;
			subresourceData.SlicePitch = subresourceData.RowPitch * height;

			UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, 1, &subresourceData);

			deferredRelease(std::move(uploadHeap));
		}

		return texture;
	}

	Resource::ImmutableCBufferPtr ResourceManager::createImmutableCBuffer(const uint32_t size, const void* const data, const bool persistent)
	{
		D3D12Resource::BufferPtr buffer = createBuffer(data, size, D3D12_RESOURCE_FLAG_NONE);

		commandList->trackAndSetResourceState(buffer.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		commandList->flushResourceBarriers();

		buffer->setStateTracking(false);

		return makeUnique<Resource::ImmutableCBuffer>(std::move(buffer), size, persistent);
	}

	Resource::DefaultCBufferPtr ResourceManager::createDefaultCBuffer(const uint32_t size, const void* const data, const bool persistent)
	{
		D3D12Resource::BufferPtr buffer = createBuffer(data, size, D3D12_RESOURCE_FLAG_NONE);

		return makeUnique<Resource::DefaultCBuffer>(std::move(buffer), size, persistent);
	}

	Resource::DefaultCBufferPtr ResourceManager::createDefaultCBuffer(const uint32_t size, const bool persistent)
	{
		D3D12Resource::BufferPtr buffer = makeUnique<D3D12Resource::Buffer>(size, true, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

		return makeUnique<Resource::DefaultCBuffer>(std::move(buffer), size, persistent);
	}

	UniquePtr<Resource::DynamicCBuffer> ResourceManager::createDynamicCBuffer(const uint32_t size, const void* const data)
	{
		UniquePtr<Resource::DynamicCBuffer> buffer = makeUnique<Resource::DynamicCBuffer>(size);

		if (data)
		{
			buffer->simpleUpdate(data);
		}

		return buffer;
	}

	Resource::BufferViewPtr ResourceManager::createTypedBufferView(const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent, const void* const data)
	{
		if (createVBV && createIBV)
		{
			LOGERROR(L"一个缓冲不能同时被用于顶点缓冲和索引缓冲！");
		}

		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if (createUAV)
		{
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12Resource::BufferPtr buffer = createBuffer(data, size, resFlags);

		if (!cpuWritable && !createUAV)
		{
			uint32_t finalState = 0;

			if (createSRV)
			{
				finalState |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
			}

			if (createVBV)
			{
				finalState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			}

			if (createIBV)
			{
				finalState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
			}

			commandList->trackAndSetResourceState(buffer.get(), finalState);

			commandList->flushResourceBarriers();

			buffer->setStateTracking(false);
		}

		return makeUnique<Resource::BufferView>(std::move(buffer), 0, format, size, createSRV, createUAV, createVBV, createIBV, cpuWritable, persistent);
	}

	Resource::BufferViewPtr ResourceManager::createTypedBufferView(const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent)
	{
		if (createVBV && createIBV)
		{
			LOGERROR(L"一个缓冲不能同时被用于顶点缓冲和索引缓冲！");
		}

		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if (createUAV)
		{
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12Resource::BufferPtr buffer = makeUnique<D3D12Resource::Buffer>(size, true, resFlags);

		return makeUnique<Resource::BufferView>(std::move(buffer), 0, format, size, createSRV, createUAV, createVBV, createIBV, cpuWritable, persistent);
	}

	Resource::BufferViewPtr ResourceManager::createStructuredBufferView(const uint32_t structureByteStride, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool cpuWritable, const bool persistent, const void* const data)
	{
		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if (createUAV)
		{
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12Resource::BufferPtr buffer = createBuffer(data, size, resFlags);

		if (!cpuWritable && !createUAV)
		{
			uint32_t finalState = 0;

			if (createSRV)
			{
				finalState |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
			}

			if (createVBV)
			{
				finalState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			}

			commandList->trackAndSetResourceState(buffer.get(), finalState);

			commandList->flushResourceBarriers();

			buffer->setStateTracking(false);
		}

		return makeUnique<Resource::BufferView>(std::move(buffer), structureByteStride, FMT::UNKNOWN, size, createSRV, createUAV, createVBV, false, cpuWritable, persistent);
	}

	Resource::BufferViewPtr ResourceManager::createStructuredBufferView(const uint32_t structureByteStride, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool cpuWritable, const bool persistent)
	{
		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if (createUAV)
		{
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12Resource::BufferPtr buffer = makeUnique<D3D12Resource::Buffer>(size, true, resFlags);

		return makeUnique<Resource::BufferView>(std::move(buffer), structureByteStride, FMT::UNKNOWN, size, createSRV, createUAV, createVBV, false, cpuWritable, persistent);
	}

	Resource::BufferViewPtr ResourceManager::createByteAddressBufferView(const uint64_t size, const bool createSRV, const bool createUAV, const bool cpuWritable, const bool persistent, const void* const data)
	{
		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if (createUAV)
		{
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12Resource::BufferPtr buffer = createBuffer(data, size, resFlags);

		if (!cpuWritable && !createUAV)
		{
			uint32_t finalState = 0;

			if (createSRV)
			{
				finalState |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
			}

			commandList->trackAndSetResourceState(buffer.get(), finalState);

			commandList->flushResourceBarriers();

			buffer->setStateTracking(false);
		}

		return makeUnique<Resource::BufferView>(std::move(buffer), 0, FMT::UNKNOWN, size, createSRV, createUAV, false, false, cpuWritable, persistent);
	}

	Resource::BufferViewPtr ResourceManager::createByteAddressBufferView(const uint64_t size, const bool createSRV, const bool createUAV, const bool cpuWritable, const bool persistent)
	{
		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if (createUAV)
		{
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12Resource::BufferPtr buffer = makeUnique<D3D12Resource::Buffer>(size, true, resFlags);

		return makeUnique<Resource::BufferView>(std::move(buffer), 0, FMT::UNKNOWN, size, createSRV, createUAV, false, false, cpuWritable, persistent);
	}

	Resource::DepthTextureViewPtr ResourceManager::createDepthTextureView(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent)
	{
		DXGI_FORMAT clearValueFormat = FMT::UNKNOWN;

		switch (resFormat)
		{
		case FMT::D32F:
		case FMT::R32TL:
			clearValueFormat = FMT::D32F;
			break;
		case FMT::D16UN:
		case FMT::R16TL:
			clearValueFormat = FMT::D16UN;
			break;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			clearValueFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
			clearValueFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		default:
			LOGERROR(L"不被支持的深度模板纹理格式！");
			break;
		}

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = clearValueFormat;
		clearValue.DepthStencil.Depth = 1.f;
		clearValue.DepthStencil.Stencil = 0;

		D3D12Resource::TexturePtr texture = makeUnique<D3D12Resource::Texture>(width, height, resFormat, arraySize, mipLevels, true, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &clearValue);

		return makeUnique<Resource::DepthTextureView>(std::move(texture), isTextureCube, persistent);
	}

	Resource::RenderTextureViewPtr ResourceManager::createRenderTextureView(const std::wstring& filePath, const bool persistent, const bool hasUAV, const bool hasRTV)
	{
		bool stateTracking = true;

		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if ((!hasRTV) && (!hasUAV))
		{
			stateTracking = false;
		}
		else
		{
			if (hasRTV)
			{
				resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}

			if (hasUAV)
			{
				resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
		}

		bool isTextureCube = false;

		D3D12Resource::TexturePtr texture = createTexture(filePath, resFlags, &isTextureCube);

		if (!stateTracking)
		{
			commandList->trackAndSetResourceState(texture.get(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

			commandList->flushResourceBarriers();

			texture->setStateTracking(false);
		}

		if (stateTracking)
		{
			return makeUnique<Resource::RenderTextureView>(std::move(texture), isTextureCube, persistent, texture->getFormat(),
				hasUAV ? texture->getFormat() : FMT::UNKNOWN, hasRTV ? texture->getFormat() : FMT::UNKNOWN);
		}
		else
		{
			return makeUnique<Resource::RenderTextureView>(std::move(texture), isTextureCube, persistent, texture->getFormat(),
				FMT::UNKNOWN, FMT::UNKNOWN);
		}
	}

	Resource::RenderTextureViewPtr ResourceManager::createRenderTextureView(const uint32_t width, const uint32_t height, const RandomDataType type, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat)
	{
		const bool hasRTV = (rtvFormat != FMT::UNKNOWN);

		const bool hasUAV = (uavFormat != FMT::UNKNOWN);

		bool stateTracking = true;

		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if ((!hasRTV) && (!hasUAV))
		{
			stateTracking = false;
		}
		else
		{
			if (hasRTV)
			{
				resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}

			if (hasUAV)
			{
				resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
		}

		D3D12Resource::TexturePtr texture = createTexture(width, height, type, resFlags);

		if (!stateTracking)
		{
			commandList->trackAndSetResourceState(texture.get(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

			commandList->flushResourceBarriers();

			texture->setStateTracking(false);
		}

		if (srvFormat == FMT::UNKNOWN)
		{
			return makeUnique<Resource::RenderTextureView>(std::move(texture), false, persistent, texture->getFormat(), FMT::UNKNOWN, FMT::UNKNOWN);
		}
		else
		{
			return makeUnique<Resource::RenderTextureView>(std::move(texture), false, persistent, srvFormat, uavFormat, rtvFormat);
		}
	}

	Resource::RenderTextureViewPtr ResourceManager::createRenderTextureView(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent, DXGI_FORMAT srvFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT rtvFormat, const float* const color)
	{
		if (FMT::UNKNOWN == srvFormat && FMT::UNKNOWN == uavFormat && FMT::UNKNOWN == rtvFormat)
		{
			srvFormat = resFormat;

			uavFormat = resFormat;

			rtvFormat = resFormat;
		}

		if (FMT::UNKNOWN == srvFormat)
		{
			LOGERROR(TOWSTRING(srvFormat), L"不能为", TOWSTRING(FMT::UNKNOWN));
		}

		const bool hasUAV = (FMT::UNKNOWN != uavFormat);

		const bool hasRTV = (FMT::UNKNOWN != rtvFormat);

		if (!hasUAV && !hasRTV)
		{
			LOGERROR(TOWSTRING(uavFormat), L"和", TOWSTRING(rtvFormat), L"其中一个必须为有效格式！");
		}

		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if (hasRTV)
		{
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}

		if (hasUAV)
		{
			resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12Resource::TexturePtr texture;

		if (color)
		{
			D3D12_CLEAR_VALUE clearValue = {};
			clearValue.Format = rtvFormat;
			memcpy(clearValue.Color, color, sizeof(float) * 4);

			texture = makeUnique<D3D12Resource::Texture>(width, height, resFormat, arraySize, mipLevels, true, resFlags, &clearValue);
		}
		else
		{
			texture = makeUnique<D3D12Resource::Texture>(width, height, resFormat, arraySize, mipLevels, true, resFlags);
		}

		return makeUnique<Resource::RenderTextureView>(std::move(texture), isTextureCube, persistent, srvFormat, uavFormat, rtvFormat);
	}

	Resource::RenderTextureViewPtr ResourceManager::createGraphicsTexture(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent, const float* const color, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvFormat)
	{
		if (FMT::UNKNOWN == srvFormat && FMT::UNKNOWN == rtvFormat)
		{
			srvFormat = resFormat;

			rtvFormat = resFormat;
		}
		else if (FMT::UNKNOWN == srvFormat && FMT::UNKNOWN != rtvFormat || FMT::UNKNOWN != srvFormat && FMT::UNKNOWN == rtvFormat)
		{
			LOGERROR(TOWSTRING(srvFormat), L"和", TOWSTRING(rtvFormat), L"要么同时为", TOWSTRING(FMT::UNKNOWN), L"，要么同时得被设置成有效值！");
		}

		return createRenderTextureView(width, height, resFormat, arraySize, mipLevels, isTextureCube, persistent, srvFormat, FMT::UNKNOWN, rtvFormat, color);
	}

	Resource::RenderTextureViewPtr ResourceManager::createComputeTexture(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent, DXGI_FORMAT srvFormat, DXGI_FORMAT uavFormat)
	{
		if (FMT::UNKNOWN == srvFormat && FMT::UNKNOWN == uavFormat)
		{
			srvFormat = resFormat;

			uavFormat = resFormat;
		}
		else if (FMT::UNKNOWN == srvFormat && FMT::UNKNOWN != uavFormat || FMT::UNKNOWN != srvFormat && FMT::UNKNOWN == uavFormat)
		{
			LOGERROR(TOWSTRING(srvFormat), L"和", TOWSTRING(uavFormat), L"要么同时为", TOWSTRING(FMT::UNKNOWN), L"，要么同时得被设置成有效值！");
		}

		return createRenderTextureView(width, height, resFormat, arraySize, mipLevels, isTextureCube, persistent, srvFormat, uavFormat, FMT::UNKNOWN);
	}

	Resource::RenderTextureViewPtr ResourceManager::createTextureCube(const std::wstring& filePath, const uint32_t texturecubeResolution, const bool persistent, const bool hasUAV, const bool hasRTV)
	{
		Resource::RenderTextureView* equirectangularMap = nullptr;

		{
			Resource::RenderTextureViewPtr equirectangularMapRet = createRenderTextureView(filePath, false, true, false);

			equirectangularMap = equirectangularMapRet.get();

			deferredRelease(std::move(equirectangularMapRet));
		}

		equirectangularMap->copyDescriptors();

		Effect::HDRClampEffect::process(*context, *equirectangularMap);

		DXGI_FORMAT resFormat = FMT::UNKNOWN;

		switch (FMT::getByteSize(equirectangularMap->getTexture()->getFormat()))
		{
		case 4:
			resFormat = FMT::RGBA8UN;
			break;
		case 8:
			resFormat = FMT::RGBA16F;
			break;
		case 16:
			resFormat = FMT::RGBA32F;
			break;
		default:
			LOGERROR(L"不被支持的等距柱状图纹理格式！");
			break;
		}

		Resource::RenderTextureView* cubeMap = nullptr;

		{
			Resource::RenderTextureViewPtr cubeMapRet = createGraphicsTexture(texturecubeResolution, texturecubeResolution, resFormat, 6, 1, true, false);

			cubeMap = cubeMapRet.get();

			deferredRelease(std::move(cubeMapRet));
		}

		Effect::LatLongMapToCubeMapEffect::process(*context, *equirectangularMap, *cubeMap);

		bool stateTracking = true;

		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if ((!hasRTV) && (!hasUAV))
		{
			stateTracking = false;
		}
		else
		{
			if (hasRTV)
			{
				resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}

			if (hasUAV)
			{
				resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
		}

		D3D12Resource::TexturePtr dstTexture = makeUnique<D3D12Resource::Texture>(texturecubeResolution, texturecubeResolution, resFormat, 6, 1, true, resFlags);

		D3D12Resource::Texture* srcTexture = cubeMap->getTexture();

		for (uint32_t i = 0; i < 6; i++)
		{
			const uint32_t dstSubresource = D3D12CalcSubresource(0, i, 0, dstTexture->getMipLevels(), dstTexture->getArraySize());

			const uint32_t srcSubresource = D3D12CalcSubresource(0, i, 0, srcTexture->getMipLevels(), srcTexture->getArraySize());

			commandList->copyTextureRegion(dstTexture.get(), dstSubresource, srcTexture, srcSubresource);
		}

		if (!stateTracking)
		{
			commandList->trackAndSetResourceState(dstTexture.get(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

			commandList->flushResourceBarriers();

			dstTexture->setStateTracking(false);
		}

		if (stateTracking)
		{
			return makeUnique<Resource::RenderTextureView>(std::move(dstTexture), true, persistent, dstTexture->getFormat(),
				hasUAV ? dstTexture->getFormat() : FMT::UNKNOWN, hasRTV ? dstTexture->getFormat() : FMT::UNKNOWN);
		}
		else
		{
			return makeUnique<Resource::RenderTextureView>(std::move(dstTexture), true, persistent, dstTexture->getFormat(),
				FMT::UNKNOWN, FMT::UNKNOWN);
		}
	}

	Resource::RenderTextureViewPtr ResourceManager::createTextureCube(const std::initializer_list<std::wstring>& texturesPath, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat)
	{
		D3D12Resource::Texture* srcTextures[6] = {};

		{
			D3D12Resource::TexturePtr srcTexturesRet[6];

			uint32_t index = 0;

			for (const std::wstring& filePath : texturesPath)
			{
				srcTexturesRet[index] = createTexture(filePath, D3D12_RESOURCE_FLAG_NONE, nullptr);

				srcTextures[index] = srcTexturesRet[index].get();

				deferredRelease(std::move(srcTexturesRet[index]));

				index++;
			}
		}

		const bool hasRTV = (rtvFormat != FMT::UNKNOWN);

		const bool hasUAV = (uavFormat != FMT::UNKNOWN);

		bool stateTracking = true;

		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

		if ((!hasRTV) && (!hasUAV))
		{
			stateTracking = false;
		}
		else
		{
			if (hasRTV)
			{
				resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}

			if (hasUAV)
			{
				resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
		}

		D3D12Resource::TexturePtr dstTexture = makeUnique<D3D12Resource::Texture>(srcTextures[0]->getWidth(), srcTextures[0]->getHeight(), srcTextures[0]->getFormat(), 6, 1, true, resFlags);

		for (uint32_t i = 0; i < 6; i++)
		{
			const uint32_t dstSubresource = D3D12CalcSubresource(0, i, 0, dstTexture->getMipLevels(), dstTexture->getArraySize());

			const uint32_t srcSubresource = D3D12CalcSubresource(0, 0, 0, srcTextures[i]->getMipLevels(), srcTextures[i]->getArraySize());

			commandList->copyTextureRegion(dstTexture.get(), dstSubresource, srcTextures[i], srcSubresource);
		}

		if (!stateTracking)
		{
			commandList->trackAndSetResourceState(dstTexture.get(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

			commandList->flushResourceBarriers();

			dstTexture->setStateTracking(false);
		}

		if (srvFormat == FMT::UNKNOWN)
		{
			return makeUnique<Resource::RenderTextureView>(std::move(dstTexture), true, persistent, dstTexture->getFormat(), FMT::UNKNOWN, FMT::UNKNOWN);
		}
		else
		{
			return makeUnique<Resource::RenderTextureView>(std::move(dstTexture), true, persistent, srvFormat, uavFormat, rtvFormat);
		}
	}

	Resource::SwapBufferPtr ResourceManager::createSwapBuffer(const std::function<Resource::BufferViewPtr(void)>& readBufferFunc, const std::function<Resource::BufferViewPtr(void)>& writeBufferFunc)
	{
		return makeUnique<Resource::SwapBuffer>(readBufferFunc, writeBufferFunc);
	}

	Resource::SwapBufferPtr ResourceManager::createSwapBuffer(const std::function<Resource::BufferViewPtr(void)>& bufferFunc)
	{
		return makeUnique<Resource::SwapBuffer>(bufferFunc, bufferFunc);
	}

	Resource::SwapTexturePtr ResourceManager::createSwapTexture(const std::function<Resource::RenderTextureViewPtr(void)>& readTextureFunc, const std::function<Resource::RenderTextureViewPtr(void)>& writeTextureFunc)
	{
		return makeUnique<Resource::SwapTexture>(readTextureFunc, writeTextureFunc);
	}

	Resource::SwapTexturePtr ResourceManager::createSwapTexture(const std::function<Resource::RenderTextureViewPtr(void)>& textureFunc)
	{
		return makeUnique<Resource::SwapTexture>(textureFunc, textureFunc);
	}
}
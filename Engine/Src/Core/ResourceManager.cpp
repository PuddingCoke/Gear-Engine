#include<Gear/Core/ResourceManager.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/GlobalEffect/HDRClampEffect.h>

#include<Gear/Core/GlobalEffect/LatLongMapToCubeMapEffect.h>

#include<Gear/Utils/Math.h>

#include<Gear/Utils/Random.h>

#include<Gear/Utils/File.h>

#include<DirectXTex/DDSTextureLoader12.h>

#include<DirectXTex/WICTextureLoader12.h>

#include<DirectXTex/DirectXTexEXR.h>

#include<DirectXTex/DirectXTex.h>

Gear::Core::ResourceManager::ResourceManager() :
	context(new GraphicsContext()), commandList(context->getCommandList()),
	d3d12Resources(new std::vector<Resource::D3D12Resource::D3D12ResourceBase*>[Graphics::getFrameBufferCount()]),
	resources(new std::vector<Resource::ResourceBase*>[Graphics::getFrameBufferCount()])
{
}

Gear::Core::ResourceManager::~ResourceManager()
{
	for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
	{
		for (const Resource::D3D12Resource::D3D12ResourceBase* const d3d12Resource : d3d12Resources[i])
		{
			delete d3d12Resource;
		}

		d3d12Resources[i].clear();

		for (const Resource::ResourceBase* const resource : resources[i])
		{
			delete resource;
		}

		resources[i].clear();
	}

	delete[] d3d12Resources;

	delete[] resources;

	delete context;
}

void Gear::Core::ResourceManager::deferredRelease(Resource::D3D12Resource::D3D12ResourceBase* const d3d12Resource)
{
	d3d12Resources[Graphics::getFrameIndex()].push_back(d3d12Resource);
}

void Gear::Core::ResourceManager::deferredRelease(Resource::ResourceBase* const resource)
{
	resources[Graphics::getFrameIndex()].push_back(resource);
}

void Gear::Core::ResourceManager::cleanTransientResources()
{
	for (const Resource::D3D12Resource::D3D12ResourceBase* const d3d12Resource : d3d12Resources[Graphics::getFrameIndex()])
	{
		delete d3d12Resource;
	}

	d3d12Resources[Graphics::getFrameIndex()].clear();

	for (const Resource::ResourceBase* const resource : resources[Graphics::getFrameIndex()])
	{
		delete resource;
	}

	resources[Graphics::getFrameIndex()].clear();
}

Gear::Core::GraphicsContext* Gear::Core::ResourceManager::getGraphicsContext() const
{
	return context;
}

Gear::Core::D3D12Core::CommandList* Gear::Core::ResourceManager::getCommandList() const
{
	return commandList;
}

Gear::Core::Resource::D3D12Resource::Buffer* Gear::Core::ResourceManager::createBuffer(const void* const data, const uint64_t size, const D3D12_RESOURCE_FLAGS resFlags)
{
	Resource::D3D12Resource::Buffer* buffer = new Resource::D3D12Resource::Buffer(size, true, resFlags);

	Resource::D3D12Resource::UploadHeap* uploadHeap = new Resource::D3D12Resource::UploadHeap(size);

	uploadHeap->update(data, size);

	deferredRelease(uploadHeap);

	commandList->copyBufferRegion(buffer, 0, uploadHeap, 0, size);

	return buffer;
}

Gear::Core::Resource::D3D12Resource::Texture* Gear::Core::ResourceManager::createTexture(const std::wstring& filePath, const D3D12_RESOURCE_FLAGS resFlags, bool* const isTextureCube)
{
	Resource::D3D12Resource::Texture* texture = nullptr;

	const std::wstring fileExtension = Utils::File::getExtension(filePath);

	if (isTextureCube)
	{
		*isTextureCube = false;
	}

	//这里要小心点，因为D3D12资源是由第三方库创建的，所以要注意资源的初始状态
	//我查看了下源码发现资源一开始其实是处于D3D12_RESOURCE_STATE_COMMON
	//但是这里有一个叫Implicit State Transitions即隐式状态转变的东西要注意
	//因为我在调试引擎的时候发现了一个奇怪的现象，搜索相关资料后找到了下面这个网站，他告诉了我答案
	//https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#implicit-state-transitions
	//资源一开始虽然是处于COMMOM状态，但是UpdateSubresources中存在CopyBufferRegion
	//这个操作执行完后，资源的状态实际上会被提升为D3D12_RESOURCE_STATE_COPY_DEST
	//这就是为什么引擎内部追踪的初始状态会被设置为D3D12_RESOURCE_STATE_COPY_DEST
	//这个地方有点坑，最好留个注释，养成好习惯
	if (fileExtension == L"bmp" || fileExtension == L"jpg" || fileExtension == L"jpeg" || fileExtension == L"png" || fileExtension == L"tiff")
	{
		std::unique_ptr<uint8_t[]> decodedData;

		D3D12_SUBRESOURCE_DATA subresource;

		ComPtr<ID3D12Resource> tex;

		CHECKERROR(DirectX::LoadWICTextureFromFileEx(GraphicsDevice::get(), filePath.c_str(), 0, resFlags, DirectX::WIC_LOADER_DEFAULT, &tex, decodedData, subresource));

		texture = new Resource::D3D12Resource::Texture(tex, true, D3D12_RESOURCE_STATE_COPY_DEST);

		const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, 1);

		Resource::D3D12Resource::UploadHeap* const uploadHeap = new Resource::D3D12Resource::UploadHeap(uploadHeapSize);

		deferredRelease(uploadHeap);

		UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, 1, &subresource);
	}
	else if (fileExtension == L"dds")
	{
		std::unique_ptr<uint8_t[]> decodedData;

		std::vector<D3D12_SUBRESOURCE_DATA> subresources;

		ComPtr<ID3D12Resource> tex;

		CHECKERROR(DirectX::LoadDDSTextureFromFileEx(GraphicsDevice::get(), filePath.c_str(), 0, resFlags, DirectX::DDS_LOADER_DEFAULT, &tex, decodedData, subresources, nullptr, isTextureCube));

		texture = new Resource::D3D12Resource::Texture(tex, true, D3D12_RESOURCE_STATE_COPY_DEST);

		const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, static_cast<uint32_t>(subresources.size()));

		Resource::D3D12Resource::UploadHeap* const uploadHeap = new Resource::D3D12Resource::UploadHeap(uploadHeapSize);

		deferredRelease(uploadHeap);

		UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, static_cast<uint32_t>(subresources.size()), subresources.data());
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

		texture = new Resource::D3D12Resource::Texture(tex, true, D3D12_RESOURCE_STATE_COPY_DEST);

		const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, 1);

		Resource::D3D12Resource::UploadHeap* const uploadHeap = new Resource::D3D12Resource::UploadHeap(uploadHeapSize);

		deferredRelease(uploadHeap);

		UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, 1, &subresource);
	}
	else
	{
		LOGERROR(fileExtension, L"is not supported");
	}

	LOGSUCCESS(L"load texture from", LogColor::brightBlue, filePath, LogColor::defaultColor, L"succeeded");

	return texture;
}

Gear::Core::Resource::D3D12Resource::Texture* Gear::Core::ResourceManager::createTexture(const uint32_t width, const uint32_t height, const RandomDataType type, const D3D12_RESOURCE_FLAGS resFlags)
{
	Resource::D3D12Resource::Texture* texture = nullptr;

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

		texture = new Resource::D3D12Resource::Texture(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, true, resFlags);

		const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, 1);

		Resource::D3D12Resource::UploadHeap* uploadHeap = new Resource::D3D12Resource::UploadHeap(uploadHeapSize);

		deferredRelease(uploadHeap);

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = colors.data();
		subresourceData.RowPitch = width * 4u;
		subresourceData.SlicePitch = subresourceData.RowPitch * height;

		UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, 1, &subresourceData);
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

		texture = new Resource::D3D12Resource::Texture(width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, 1, true, resFlags);

		const uint64_t uploadHeapSize = GetRequiredIntermediateSize(texture->getResource(), 0, 1);

		Resource::D3D12Resource::UploadHeap* uploadHeap = new Resource::D3D12Resource::UploadHeap(uploadHeapSize);

		deferredRelease(uploadHeap);

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = colors.data();
		subresourceData.RowPitch = width * 8u;
		subresourceData.SlicePitch = subresourceData.RowPitch * height;

		UpdateSubresources(commandList->get(), texture->getResource(), uploadHeap->getResource(), 0, 0, 1, &subresourceData);
	}

	return texture;
}

Gear::Core::Resource::ImmutableCBuffer* Gear::Core::ResourceManager::createImmutableCBuffer(const uint32_t size, const void* const data, const bool persistent)
{
	Resource::D3D12Resource::Buffer* const buffer = createBuffer(data, size, D3D12_RESOURCE_FLAG_NONE);

	commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	commandList->transitionResources();

	buffer->setStateTracking(false);

	return new Resource::ImmutableCBuffer(buffer, size, persistent);
}

Gear::Core::Resource::StaticCBuffer* Gear::Core::ResourceManager::createStaticCBuffer(const uint32_t size, const void* const data, const bool persistent)
{
	Resource::D3D12Resource::Buffer* const buffer = createBuffer(data, size, D3D12_RESOURCE_FLAG_NONE);

	return new Resource::StaticCBuffer(buffer, size, persistent);
}

Gear::Core::Resource::StaticCBuffer* Gear::Core::ResourceManager::createStaticCBuffer(const uint32_t size, const bool persistent)
{
	Resource::D3D12Resource::Buffer* const buffer = new Resource::D3D12Resource::Buffer(size, true, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

	return new Resource::StaticCBuffer(buffer, size, persistent);
}

Gear::Core::Resource::DynamicCBuffer* Gear::Core::ResourceManager::createDynamicCBuffer(const uint32_t size, const void* const data)
{
	Resource::DynamicCBuffer* const buffer = new Resource::DynamicCBuffer(size);

	if (data)
	{
		buffer->simpleUpdate(data);
	}

	return buffer;
}

Gear::Core::Resource::BufferView* Gear::Core::ResourceManager::createTypedBufferView(const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent, const void* const data)
{
	if (createVBV && createIBV)
	{
		LOGERROR(L"a buffer cannot be used as VBV and IBV at the same time");
	}

	D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

	if (createUAV)
	{
		resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	Resource::D3D12Resource::Buffer* const buffer = createBuffer(data, size, resFlags);

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

		commandList->trackAndSetResourceState(buffer, finalState);

		commandList->transitionResources();

		buffer->setStateTracking(false);
	}

	return new Resource::BufferView(buffer, 0, format, size, createSRV, createUAV, createVBV, createIBV, cpuWritable, persistent);
}

Gear::Core::Resource::BufferView* Gear::Core::ResourceManager::createTypedBufferView(const DXGI_FORMAT format, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool createIBV, const bool cpuWritable, const bool persistent)
{
	if (createVBV && createIBV)
	{
		LOGERROR(L"a bufffer cannot be used as VBV and IBV at the same time");
	}

	D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

	if (createUAV)
	{
		resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	Resource::D3D12Resource::Buffer* const buffer = new Resource::D3D12Resource::Buffer(size, true, resFlags);

	return new Resource::BufferView(buffer, 0, format, size, createSRV, createUAV, createVBV, createIBV, cpuWritable, persistent);
}

Gear::Core::Resource::BufferView* Gear::Core::ResourceManager::createStructuredBufferView(const uint32_t structureByteStride, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool cpuWritable, const bool persistent, const void* const data)
{
	D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

	if (createUAV)
	{
		resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	Resource::D3D12Resource::Buffer* const buffer = createBuffer(data, size, resFlags);

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

		commandList->trackAndSetResourceState(buffer, finalState);

		commandList->transitionResources();

		buffer->setStateTracking(false);
	}

	return new Resource::BufferView(buffer, structureByteStride, DXGI_FORMAT_UNKNOWN, size, createSRV, createUAV, createVBV, false, cpuWritable, persistent);
}

Gear::Core::Resource::BufferView* Gear::Core::ResourceManager::createStructuredBufferView(const uint32_t structureByteStride, const uint64_t size, const bool createSRV, const bool createUAV, const bool createVBV, const bool cpuWritable, const bool persistent)
{
	D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

	if (createUAV)
	{
		resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	Resource::D3D12Resource::Buffer* const buffer = new Resource::D3D12Resource::Buffer(size, true, resFlags);

	return new Resource::BufferView(buffer, structureByteStride, DXGI_FORMAT_UNKNOWN, size, createSRV, createUAV, createVBV, false, cpuWritable, persistent);
}

Gear::Core::Resource::BufferView* Gear::Core::ResourceManager::createByteAddressBufferView(const uint64_t size, const bool createSRV, const bool createUAV, const bool cpuWritable, const bool persistent, const void* const data)
{
	D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

	if (createUAV)
	{
		resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	Resource::D3D12Resource::Buffer* const buffer = createBuffer(data, size, resFlags);

	if (!cpuWritable && !createUAV)
	{
		uint32_t finalState = 0;

		if (createSRV)
		{
			finalState |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
		}

		commandList->trackAndSetResourceState(buffer, finalState);

		commandList->transitionResources();

		buffer->setStateTracking(false);
	}

	return new Resource::BufferView(buffer, 0, DXGI_FORMAT_UNKNOWN, size, createSRV, createUAV, false, false, cpuWritable, persistent);
}

Gear::Core::Resource::BufferView* Gear::Core::ResourceManager::createByteAddressBufferView(const uint64_t size, const bool createSRV, const bool createUAV, const bool cpuWritable, const bool persistent)
{
	D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;

	if (createUAV)
	{
		resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	Resource::D3D12Resource::Buffer* const buffer = new Resource::D3D12Resource::Buffer(size, true, resFlags);

	return new Resource::BufferView(buffer, 0, DXGI_FORMAT_UNKNOWN, size, createSRV, createUAV, false, false, cpuWritable, persistent);
}

Gear::Core::Resource::TextureDepthView* Gear::Core::ResourceManager::createTextureDepthView(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent)
{
	DXGI_FORMAT clearValueFormat = DXGI_FORMAT_UNKNOWN;

	switch (resFormat)
	{
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_TYPELESS:
		clearValueFormat = DXGI_FORMAT_D32_FLOAT;
		break;
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_TYPELESS:
		clearValueFormat = DXGI_FORMAT_D16_UNORM;
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
		LOGERROR(L"not supported dsv format");
		break;
	}

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = clearValueFormat;
	clearValue.DepthStencil.Depth = 1.f;
	clearValue.DepthStencil.Stencil = 0;

	Resource::D3D12Resource::Texture* texture = new Resource::D3D12Resource::Texture(width, height, resFormat, arraySize, mipLevels, true, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &clearValue);

	return new Resource::TextureDepthView(texture, isTextureCube, persistent);
}

Gear::Core::Resource::TextureRenderView* Gear::Core::ResourceManager::createTextureRenderView(const std::wstring& filePath, const bool persistent, const bool hasUAV, const bool hasRTV)
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

	Resource::D3D12Resource::Texture* const texture = createTexture(filePath, resFlags, &isTextureCube);

	if (!stateTracking)
	{
		commandList->trackAndSetResourceState(texture, Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		commandList->transitionResources();

		texture->setStateTracking(false);
	}

	if (stateTracking)
	{
		return new Resource::TextureRenderView(texture, isTextureCube, persistent, texture->getFormat(),
			hasUAV ? texture->getFormat() : DXGI_FORMAT_UNKNOWN, hasRTV ? texture->getFormat() : DXGI_FORMAT_UNKNOWN);
	}
	else
	{
		return new Resource::TextureRenderView(texture, isTextureCube, persistent, texture->getFormat(),
			DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	}
}

Gear::Core::Resource::TextureRenderView* Gear::Core::ResourceManager::createTextureRenderView(const uint32_t width, const uint32_t height, const RandomDataType type, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat)
{
	const bool hasRTV = (rtvFormat != DXGI_FORMAT_UNKNOWN);

	const bool hasUAV = (uavFormat != DXGI_FORMAT_UNKNOWN);

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

	Resource::D3D12Resource::Texture* texture = createTexture(width, height, type, resFlags);

	if (!stateTracking)
	{
		commandList->trackAndSetResourceState(texture, Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		commandList->transitionResources();

		texture->setStateTracking(false);
	}

	if (srvFormat == DXGI_FORMAT_UNKNOWN)
	{
		return new Resource::TextureRenderView(texture, false, persistent, texture->getFormat(), DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	}
	else
	{
		return new Resource::TextureRenderView(texture, false, persistent, srvFormat, uavFormat, rtvFormat);
	}
}

Gear::Core::Resource::TextureRenderView* Gear::Core::ResourceManager::createTextureRenderView(const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat, const float* const color)
{
	const bool hasRTV = (rtvFormat != DXGI_FORMAT_UNKNOWN);

	const bool hasUAV = (uavFormat != DXGI_FORMAT_UNKNOWN);

	if ((!hasRTV) && (!hasUAV))
	{
		LOGERROR(L"you must set UAV or RTV format for customized render texture view");
	}
	else if (srvFormat == DXGI_FORMAT_UNKNOWN)
	{
		LOGERROR(L"customized render texture view must have a valid SRV format");
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

	Resource::D3D12Resource::Texture* texture = nullptr;

	if (color)
	{
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = rtvFormat;
		memcpy(clearValue.Color, color, sizeof(float) * 4);

		texture = new Resource::D3D12Resource::Texture(width, height, resFormat, arraySize, mipLevels, true, resFlags, &clearValue);
	}
	else
	{
		texture = new Resource::D3D12Resource::Texture(width, height, resFormat, arraySize, mipLevels, true, resFlags);
	}

	return new Resource::TextureRenderView(texture, isTextureCube, persistent, srvFormat, uavFormat, rtvFormat);
}

Gear::Core::Resource::TextureRenderView* Gear::Core::ResourceManager::createTextureCube(const std::wstring& filePath, const uint32_t texturecubeResolution, const bool persistent, const bool hasUAV, const bool hasRTV)
{
	Resource::TextureRenderView* const equirectangularMap = createTextureRenderView(filePath, false, true, false);

	equirectangularMap->copyDescriptors();

	deferredRelease(equirectangularMap);

	GlobalEffect::HDRClampEffect::process(context, equirectangularMap);

	DXGI_FORMAT resFormat = DXGI_FORMAT_UNKNOWN;

	switch (FMT::getByteSize(equirectangularMap->getTexture()->getFormat()))
	{
	case 4:
		resFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case 8:
		resFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case 16:
		resFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		LOGERROR(L"not supported equirectangular texture format");
		break;
	}

	Resource::TextureRenderView* const cubeMap = createTextureRenderView(texturecubeResolution, texturecubeResolution, resFormat, 6, 1, true, false,
		resFormat, DXGI_FORMAT_UNKNOWN, resFormat);

	deferredRelease(cubeMap);

	GlobalEffect::LatLongMapToCubeMapEffect::process(context, equirectangularMap, cubeMap);

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

	Resource::D3D12Resource::Texture* dstTexture = new Resource::D3D12Resource::Texture(texturecubeResolution, texturecubeResolution, resFormat, 6, 1, true, resFlags);

	Resource::D3D12Resource::Texture* srcTexture = cubeMap->getTexture();

	for (uint32_t i = 0; i < 6; i++)
	{
		const uint32_t dstSubresource = D3D12CalcSubresource(0, i, 0, dstTexture->getMipLevels(), dstTexture->getArraySize());

		const uint32_t srcSubresource = D3D12CalcSubresource(0, i, 0, srcTexture->getMipLevels(), srcTexture->getArraySize());

		commandList->copyTextureRegion(dstTexture, dstSubresource, srcTexture, srcSubresource);
	}

	if (!stateTracking)
	{
		commandList->trackAndSetResourceState(dstTexture, Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		commandList->transitionResources();

		dstTexture->setStateTracking(false);
	}

	if (stateTracking)
	{
		return new Resource::TextureRenderView(dstTexture, true, persistent, dstTexture->getFormat(),
			hasUAV ? dstTexture->getFormat() : DXGI_FORMAT_UNKNOWN, hasRTV ? dstTexture->getFormat() : DXGI_FORMAT_UNKNOWN);
	}
	else
	{
		return new Resource::TextureRenderView(dstTexture, true, persistent, dstTexture->getFormat(),
			DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	}
}

Gear::Core::Resource::TextureRenderView* Gear::Core::ResourceManager::createTextureCube(const std::initializer_list<std::wstring>& texturesPath, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat)
{
	Resource::D3D12Resource::Texture* srcTextures[6] = {};

	{
		uint32_t index = 0;

		for (const std::wstring& filePath : texturesPath)
		{
			srcTextures[index] = createTexture(filePath, D3D12_RESOURCE_FLAG_NONE, nullptr);

			deferredRelease(srcTextures[index]);

			index++;
		}
	}

	const bool hasRTV = (rtvFormat != DXGI_FORMAT_UNKNOWN);

	const bool hasUAV = (uavFormat != DXGI_FORMAT_UNKNOWN);

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

	Resource::D3D12Resource::Texture* dstTexture = new Resource::D3D12Resource::Texture(srcTextures[0]->getWidth(), srcTextures[0]->getHeight(), srcTextures[0]->getFormat(), 6, 1, true, resFlags);

	for (uint32_t i = 0; i < 6; i++)
	{
		const uint32_t dstSubresource = D3D12CalcSubresource(0, i, 0, dstTexture->getMipLevels(), dstTexture->getArraySize());

		const uint32_t srcSubresource = D3D12CalcSubresource(0, 0, 0, srcTextures[i]->getMipLevels(), srcTextures[i]->getArraySize());

		commandList->copyTextureRegion(dstTexture, dstSubresource, srcTextures[i], srcSubresource);
	}

	if (!stateTracking)
	{
		commandList->trackAndSetResourceState(dstTexture, Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		commandList->transitionResources();

		dstTexture->setStateTracking(false);
	}

	if (srvFormat == DXGI_FORMAT_UNKNOWN)
	{
		return new Resource::TextureRenderView(dstTexture, true, persistent, dstTexture->getFormat(), DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	}
	else
	{
		return new Resource::TextureRenderView(dstTexture, true, persistent, srvFormat, uavFormat, rtvFormat);
	}
}

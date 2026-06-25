#include<Gear/Resource/DepthTextureView.h>

#include<Gear/Core/FMT.h>

namespace Gear::Resource
{
	DepthTextureView::DepthTextureView(D3D12Resource::TexturePtr texturePtr, const bool isTextureCube, const bool persistent) :
		ResourceBase(persistent),
		depthSRVFormat(FMT::UNKNOWN),
		stencilSRVFormat(FMT::UNKNOWN),
		dsvFormat(FMT::UNKNOWN),
		allDepthSRVIndex(makeShared<uint32_t>()),
		allStencilSRVIndex(makeShared<uint32_t>()),
		depthSRVMipIndices(makeShared<std::vector<uint32_t>>()),
		depthSRVMipGPUHandles(makeShared<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>>()),
		stencilSRVMipIndices(makeShared<std::vector<uint32_t>>()),
		dsvMipHandles(makeShared<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>>()),
		texture(std::move(texturePtr))
	{
		const uint32_t mipLevels = texture->getMipLevels();

		const uint32_t arraySize = texture->getArraySize();

		switch (texture->getFormat())
		{
		case FMT::D32F:
		case FMT::D16UN:
		case FMT::D32FS8X24UI:
		case FMT::D24UNS8UI:
			dsvFormat = texture->getFormat();
			break;
		case FMT::R32TL:
			depthSRVFormat = FMT::R32F;
			dsvFormat = FMT::D32F;
			break;
		case FMT::R16TL:
			depthSRVFormat = FMT::R16UN;
			dsvFormat = FMT::D16UN;
			break;
		case FMT::R32G8X24TL:
			depthSRVFormat = FMT::R32FX8X24TL;
			stencilSRVFormat = FMT::X32TLG8X24UI;
			dsvFormat = FMT::D32FS8X24UI;
			break;
		case FMT::R24G8TL:
			depthSRVFormat = FMT::R24UNX8TL;
			stencilSRVFormat = FMT::X24TLG8UI;
			dsvFormat = FMT::D24UNS8UI;
			break;
		default:
			LOGERROR(L"不支持的深度模板纹理格式！");
			break;
		}

		const bool hasDepthSRV = (FMT::UNKNOWN != depthSRVFormat);

		const bool hasStencilSRV = (FMT::UNKNOWN != stencilSRVFormat);

		//创建DSV
		{
			D3D12Core::DescriptorHandle descriptorHandle;

			if (persistent)
			{
				descriptorHandle = LocalDescriptorHeap::getDepthStencilHeap()->allocStaticDescriptor(mipLevels);
			}
			else
			{
				descriptorHandle = LocalDescriptorHeap::getDepthStencilHeap()->allocDynamicDescriptor(mipLevels);
			}

			(*dsvMipHandles).resize(mipLevels);

			for (uint32_t i = 0; i < mipLevels; i++)
			{
				(*dsvMipHandles)[i] = descriptorHandle.getCurrentCPUHandle();

				D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

				if (arraySize > 1)
				{
					desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					desc.Format = dsvFormat;
					desc.Texture2DArray.ArraySize = arraySize;
					desc.Texture2DArray.FirstArraySlice = 0;
					desc.Texture2DArray.MipSlice = i;
				}
				else
				{
					desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					desc.Format = dsvFormat;
					desc.Texture2D.MipSlice = i;
				}

				GraphicsDevice::get()->CreateDepthStencilView(texture->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

				descriptorHandle.move();
			}
		}

		setNumCBVSRVUAVDescriptors((static_cast<uint32_t>(hasDepthSRV) + static_cast<uint32_t>(hasStencilSRV)) * (1 + mipLevels));

		if (getNumCBVSRVUAVDescriptors())
		{
			D3D12Core::DescriptorHandle descriptorHandle = allocCBVSRVUAVDescriptors();

			if (hasDepthSRV)
			{
				//创建能访问所有mipslice的SRV
				{
					*allDepthSRVIndex = descriptorHandle.getCurrentIndex();

					D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

					if (isTextureCube)
					{
						const uint32_t numCube = arraySize / 6;

						if (numCube > 1)
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
							desc.Format = depthSRVFormat;
							desc.TextureCubeArray.First2DArrayFace = 0;
							desc.TextureCubeArray.MipLevels = mipLevels;
							desc.TextureCubeArray.MostDetailedMip = 0;
							desc.TextureCubeArray.NumCubes = numCube;
							desc.TextureCubeArray.ResourceMinLODClamp = 0.f;
						}
						else
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
							desc.Format = depthSRVFormat;
							desc.TextureCube.MipLevels = mipLevels;
							desc.TextureCube.MostDetailedMip = 0;
							desc.TextureCube.ResourceMinLODClamp = 0.f;
						}
					}
					else
					{
						if (arraySize > 1)
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
							desc.Format = depthSRVFormat;
							desc.Texture2DArray.ArraySize = arraySize;
							desc.Texture2DArray.FirstArraySlice = 0;
							desc.Texture2DArray.MipLevels = mipLevels;
							desc.Texture2DArray.MostDetailedMip = 0;
							desc.Texture2DArray.PlaneSlice = 0;
							desc.Texture2DArray.ResourceMinLODClamp = 0.f;
						}
						else
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
							desc.Format = depthSRVFormat;
							desc.Texture2D.MipLevels = mipLevels;
							desc.Texture2D.MostDetailedMip = 0;
							desc.Texture2D.PlaneSlice = 0;
							desc.Texture2D.ResourceMinLODClamp = 0.f;
						}
					}

					GraphicsDevice::get()->CreateShaderResourceView(texture->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

					descriptorHandle.move();
				}

				(*depthSRVMipIndices).resize(mipLevels);

				(*depthSRVMipGPUHandles).resize(mipLevels);

				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					(*depthSRVMipIndices)[i] = descriptorHandle.getCurrentIndex();

					(*depthSRVMipGPUHandles)[i] = descriptorHandle.getCurrentGPUHandle();

					D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

					if (isTextureCube)
					{
						const uint32_t numCube = arraySize / 6;

						if (numCube > 1)
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
							desc.Format = depthSRVFormat;
							desc.TextureCubeArray.First2DArrayFace = 0;
							desc.TextureCubeArray.MipLevels = 1;
							desc.TextureCubeArray.MostDetailedMip = i;
							desc.TextureCubeArray.NumCubes = numCube;
							desc.TextureCubeArray.ResourceMinLODClamp = 0.f;
						}
						else
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
							desc.Format = depthSRVFormat;
							desc.TextureCube.MipLevels = 1;
							desc.TextureCube.MostDetailedMip = i;
							desc.TextureCube.ResourceMinLODClamp = 0.f;
						}
					}
					else
					{
						if (arraySize > 1)
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
							desc.Format = depthSRVFormat;
							desc.Texture2DArray.ArraySize = arraySize;
							desc.Texture2DArray.FirstArraySlice = 0;
							desc.Texture2DArray.MipLevels = 1;
							desc.Texture2DArray.MostDetailedMip = i;
							desc.Texture2DArray.PlaneSlice = 0;
							desc.Texture2DArray.ResourceMinLODClamp = 0.f;
						}
						else
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
							desc.Format = depthSRVFormat;
							desc.Texture2D.MipLevels = 1;
							desc.Texture2D.MostDetailedMip = i;
							desc.Texture2D.PlaneSlice = 0;
							desc.Texture2D.ResourceMinLODClamp = 0.f;
						}
					}

					GraphicsDevice::get()->CreateShaderResourceView(texture->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

					descriptorHandle.move();
				}
			}

			if (hasStencilSRV)
			{
				//创建能访问所有mipslice的SRV
				{
					*allStencilSRVIndex = descriptorHandle.getCurrentIndex();

					D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

					if (isTextureCube)
					{
						const uint32_t numCube = arraySize / 6;

						if (numCube > 1)
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
							desc.Format = stencilSRVFormat;
							desc.TextureCubeArray.First2DArrayFace = 0;
							desc.TextureCubeArray.MipLevels = mipLevels;
							desc.TextureCubeArray.MostDetailedMip = 0;
							desc.TextureCubeArray.NumCubes = numCube;
							desc.TextureCubeArray.ResourceMinLODClamp = 0.f;
						}
						else
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
							desc.Format = stencilSRVFormat;
							desc.TextureCube.MipLevels = mipLevels;
							desc.TextureCube.MostDetailedMip = 0;
							desc.TextureCube.ResourceMinLODClamp = 0.f;
						}
					}
					else
					{
						if (arraySize > 1)
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
							desc.Format = stencilSRVFormat;
							desc.Texture2DArray.ArraySize = arraySize;
							desc.Texture2DArray.FirstArraySlice = 0;
							desc.Texture2DArray.MipLevels = mipLevels;
							desc.Texture2DArray.MostDetailedMip = 0;
							desc.Texture2DArray.PlaneSlice = 0;
							desc.Texture2DArray.ResourceMinLODClamp = 0.f;
						}
						else
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
							desc.Format = stencilSRVFormat;
							desc.Texture2D.MipLevels = mipLevels;
							desc.Texture2D.MostDetailedMip = 0;
							desc.Texture2D.PlaneSlice = 0;
							desc.Texture2D.ResourceMinLODClamp = 0.f;
						}
					}

					GraphicsDevice::get()->CreateShaderResourceView(texture->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

					descriptorHandle.move();
				}

				(*stencilSRVMipIndices).resize(mipLevels);

				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					(*stencilSRVMipIndices)[i] = descriptorHandle.getCurrentIndex();

					D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

					if (isTextureCube)
					{
						const uint32_t numCube = arraySize / 6;

						if (numCube > 1)
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
							desc.Format = stencilSRVFormat;
							desc.TextureCubeArray.First2DArrayFace = 0;
							desc.TextureCubeArray.MipLevels = 1;
							desc.TextureCubeArray.MostDetailedMip = i;
							desc.TextureCubeArray.NumCubes = numCube;
							desc.TextureCubeArray.ResourceMinLODClamp = 0.f;
						}
						else
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
							desc.Format = stencilSRVFormat;
							desc.TextureCube.MipLevels = 1;
							desc.TextureCube.MostDetailedMip = i;
							desc.TextureCube.ResourceMinLODClamp = 0.f;
						}
					}
					else
					{
						if (arraySize > 1)
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
							desc.Format = stencilSRVFormat;
							desc.Texture2DArray.ArraySize = arraySize;
							desc.Texture2DArray.FirstArraySlice = 0;
							desc.Texture2DArray.MipLevels = 1;
							desc.Texture2DArray.MostDetailedMip = i;
							desc.Texture2DArray.PlaneSlice = 0;
							desc.Texture2DArray.ResourceMinLODClamp = 0.f;
						}
						else
						{
							desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
							desc.Format = stencilSRVFormat;
							desc.Texture2D.MipLevels = 1;
							desc.Texture2D.MostDetailedMip = i;
							desc.Texture2D.PlaneSlice = 0;
							desc.Texture2D.ResourceMinLODClamp = 0.f;
						}
					}

					GraphicsDevice::get()->CreateShaderResourceView(texture->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

					descriptorHandle.move();
				}
			}
		}
	}

	DepthTextureView::DepthTextureView(const DepthTextureView& dtv) :
		ResourceBase(dtv),
		depthSRVFormat(dtv.depthSRVFormat),
		stencilSRVFormat(dtv.stencilSRVFormat),
		dsvFormat(dtv.dsvFormat),
		allDepthSRVIndex(dtv.allDepthSRVIndex),
		allStencilSRVIndex(dtv.allStencilSRVIndex),
		depthSRVMipIndices(dtv.depthSRVMipIndices),
		depthSRVMipGPUHandles(dtv.depthSRVMipGPUHandles),
		stencilSRVMipIndices(dtv.stencilSRVMipIndices),
		dsvMipHandles(dtv.dsvMipHandles),
		texture(dtv.texture ? makeUnique<D3D12Resource::Texture>(*dtv.texture) : nullptr)
	{
	}

	DepthTextureView::~DepthTextureView()
	{
	}

	ShaderResourceDesc DepthTextureView::getAllDepthIndex() const
	{
		const ShaderResourceDesc desc = {
		.type = ShaderResourceDesc::TEXTURE,
		.state = ShaderResourceDesc::SRV,
		.resourceIndex = allDepthSRVIndex.get(),
		.textureDesc = {
				.texture = texture.get(),
				.mipSlice = D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS
		} };

		return desc;
	}

	ShaderResourceDesc DepthTextureView::getAllStencilIndex() const
	{
		const ShaderResourceDesc desc = {
			.type = ShaderResourceDesc::TEXTURE,
			.state = ShaderResourceDesc::SRV,
			.resourceIndex = allStencilSRVIndex.get(),
			.textureDesc = {
				.texture = texture.get(),
				.mipSlice = D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS
		} };

		return desc;
	}

	ShaderResourceDesc DepthTextureView::getDepthMipIndex(const uint32_t mipSlice) const
	{
		const ShaderResourceDesc desc = {
		.type = ShaderResourceDesc::TEXTURE,
		.state = ShaderResourceDesc::SRV,
		.resourceIndex = &(*depthSRVMipIndices)[mipSlice],
		.textureDesc = {
				.texture = texture.get(),
				.mipSlice = mipSlice
		} };

		return desc;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DepthTextureView::getDepthMipGPUHandle(const uint32_t mipSlice) const
	{
		return (*depthSRVMipGPUHandles)[mipSlice];
	}

	ShaderResourceDesc DepthTextureView::getStencilMipIndex(const uint32_t mipSlice) const
	{
		const ShaderResourceDesc desc = {
		.type = ShaderResourceDesc::TEXTURE,
		.state = ShaderResourceDesc::SRV,
		.resourceIndex = &(*stencilSRVMipIndices)[mipSlice],
		.textureDesc = {
				.texture = texture.get(),
				.mipSlice = mipSlice
		} };

		return desc;
	}

	DepthStencilDesc DepthTextureView::getDSVMip(const uint32_t mipSlice) const
	{
		const DepthStencilDesc desc = {
		.texture = texture.get(),
		.mipSlice = mipSlice,
		.dsvHandle = (*dsvMipHandles)[mipSlice],
		.dsvFormat = getDSVFormat()
		};

		return desc;
	}

	D3D12Resource::Texture* DepthTextureView::getTexture() const
	{
		return texture.get();
	}

	bool DepthTextureView::copyDescriptors()
	{
		D3D12Core::DescriptorHandle shaderVisibleHandle;

		const bool copied = copyToResourceHeap(shaderVisibleHandle);

		if (copied)
		{
			if (FMT::UNKNOWN != depthSRVFormat)
			{
				*allDepthSRVIndex = shaderVisibleHandle.getCurrentIndex();

				shaderVisibleHandle.move();

				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					(*depthSRVMipIndices)[i] = shaderVisibleHandle.getCurrentIndex();

					(*depthSRVMipGPUHandles)[i] = shaderVisibleHandle.getCurrentGPUHandle();

					shaderVisibleHandle.move();
				}
			}

			if (FMT::UNKNOWN != stencilSRVFormat)
			{
				*allStencilSRVIndex = shaderVisibleHandle.getCurrentIndex();

				shaderVisibleHandle.move();

				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					(*stencilSRVMipIndices)[i] = shaderVisibleHandle.getCurrentIndex();

					shaderVisibleHandle.move();
				}
			}
		}

		return copied;
	}

	DXGI_FORMAT DepthTextureView::getDSVFormat() const
	{
		return dsvFormat;
	}
}
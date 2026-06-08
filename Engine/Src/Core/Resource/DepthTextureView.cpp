#include<Gear/Core/Resource/DepthTextureView.h>

namespace Gear::Core::Resource
{
	DepthTextureView::DepthTextureView(D3D12Resource::TexturePtr texturePtr, const bool isTextureCube, const bool persistent) :
		ResourceBase(persistent), texture(std::move(texturePtr)), allDepthSRVIndex(0), allStencilSRVIndex(0)
	{
		const uint32_t mipLevels = texture->getMipLevels();
		const uint32_t arraySize = texture->getArraySize();
		const DXGI_FORMAT resFormat = texture->getFormat();

		DXGI_FORMAT depthSRVFormat = FMT::UNKNOWN;
		DXGI_FORMAT stencilSRVFormat = FMT::UNKNOWN;
		DXGI_FORMAT dsvFormat = FMT::UNKNOWN;

		switch (resFormat)
		{
		case FMT::D32F:
			dsvFormat = FMT::D32F;
			break;
		case FMT::D16UN:
			dsvFormat = FMT::D16UN;
			break;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			dsvFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case FMT::R32TL:
			depthSRVFormat = FMT::R32F;
			dsvFormat = FMT::D32F;
			break;
		case FMT::R16TL:
			depthSRVFormat = FMT::R16UN;
			dsvFormat = FMT::D16UN;
			break;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			depthSRVFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			stencilSRVFormat = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
			dsvFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			break;
		case DXGI_FORMAT_R24G8_TYPELESS:
			depthSRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			stencilSRVFormat = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		default:
			LOGERROR(L"不支持的深度模板纹理格式！");
			break;
		}

		hasDepthSRV = (depthSRVFormat != FMT::UNKNOWN);

		hasStencilSRV = (stencilSRVFormat != FMT::UNKNOWN);

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

			dsvMipHandles.resize(mipLevels);

			for (uint32_t i = 0; i < mipLevels; i++)
			{
				dsvMipHandles[i] = descriptorHandle.getCurrentCPUHandle();

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
					allDepthSRVIndex = descriptorHandle.getCurrentIndex();

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

				depthSRVMipIndices.resize(mipLevels);

				depthSRVMipGPUHandles.resize(mipLevels);

				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					depthSRVMipIndices[i] = descriptorHandle.getCurrentIndex();

					depthSRVMipGPUHandles[i] = descriptorHandle.getCurrentGPUHandle();

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
					allStencilSRVIndex = descriptorHandle.getCurrentIndex();

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

				stencilSRVMipIndices.resize(mipLevels);

				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					stencilSRVMipIndices[i] = descriptorHandle.getCurrentIndex();

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

	DepthTextureView::DepthTextureView(const DepthTextureView& tdv) :
		ResourceBase(tdv.persistent),
		hasDepthSRV(tdv.hasDepthSRV),
		hasStencilSRV(tdv.hasStencilSRV),
		allDepthSRVIndex(tdv.allDepthSRVIndex),
		allStencilSRVIndex(tdv.allStencilSRVIndex),
		depthSRVMipIndices(tdv.depthSRVMipIndices),
		stencilSRVMipIndices(tdv.stencilSRVMipIndices),
		dsvMipHandles(tdv.dsvMipHandles),
		texture(makeUnique<D3D12Resource::Texture>(*tdv.texture))
	{
	}

	DepthTextureView::~DepthTextureView()
	{
	}

	D3D12Resource::ShaderResourceDesc DepthTextureView::getAllDepthIndex() const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::TEXTURE;
		desc.state = D3D12Resource::ShaderResourceDesc::SRV;
		desc.resourceIndex = allDepthSRVIndex;
		desc.textureDesc.texture = texture.get();
		desc.textureDesc.mipSlice = D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS;

		return desc;
	}

	D3D12Resource::ShaderResourceDesc DepthTextureView::getAllStencilIndex() const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::TEXTURE;
		desc.state = D3D12Resource::ShaderResourceDesc::SRV;
		desc.resourceIndex = allStencilSRVIndex;
		desc.textureDesc.texture = texture.get();
		desc.textureDesc.mipSlice = D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS;

		return desc;
	}

	D3D12Resource::ShaderResourceDesc DepthTextureView::getDepthMipIndex(const uint32_t mipSlice) const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::TEXTURE;
		desc.state = D3D12Resource::ShaderResourceDesc::SRV;
		desc.resourceIndex = depthSRVMipIndices[mipSlice];
		desc.textureDesc.texture = texture.get();
		desc.textureDesc.mipSlice = mipSlice;

		return desc;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DepthTextureView::getDepthMipGPUHandle(const uint32_t mipSlice) const
	{
		return depthSRVMipGPUHandles[mipSlice];
	}

	D3D12Resource::ShaderResourceDesc DepthTextureView::getStencilMipIndex(const uint32_t mipSlice) const
	{
		D3D12Resource::ShaderResourceDesc desc = {};
		desc.type = D3D12Resource::ShaderResourceDesc::TEXTURE;
		desc.state = D3D12Resource::ShaderResourceDesc::SRV;
		desc.resourceIndex = stencilSRVMipIndices[mipSlice];
		desc.textureDesc.texture = texture.get();
		desc.textureDesc.mipSlice = mipSlice;

		return desc;
	}

	D3D12Resource::DepthStencilDesc DepthTextureView::getDSVMipHandle(const uint32_t mipSlice) const
	{
		D3D12Resource::DepthStencilDesc desc = {};
		desc.texture = texture.get();
		desc.mipSlice = mipSlice;
		desc.dsvHandle = dsvMipHandles[mipSlice];

		return desc;
	}

	D3D12Resource::Texture* DepthTextureView::getTexture() const
	{
		return texture.get();
	}

	void DepthTextureView::copyDescriptors()
	{
		D3D12Core::DescriptorHandle shaderVisibleHandle = copyToResourceHeap();

		if (hasDepthSRV)
		{
			allDepthSRVIndex = shaderVisibleHandle.getCurrentIndex();

			shaderVisibleHandle.move();

			for (uint32_t i = 0; i < texture->getMipLevels(); i++)
			{
				depthSRVMipIndices[i] = shaderVisibleHandle.getCurrentIndex();

				shaderVisibleHandle.move();
			}
		}

		if (hasStencilSRV)
		{
			allStencilSRVIndex = shaderVisibleHandle.getCurrentIndex();

			shaderVisibleHandle.move();

			for (uint32_t i = 0; i < texture->getMipLevels(); i++)
			{
				stencilSRVMipIndices[i] = shaderVisibleHandle.getCurrentIndex();

				shaderVisibleHandle.move();
			}
		}
	}
}
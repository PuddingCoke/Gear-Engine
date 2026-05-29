#include<Gear/Core/Resource/TextureRenderView.h>

Gear::Core::Resource::TextureRenderView::TextureRenderView(D3D12Resource::Texture* const texture, const bool isTextureCube, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat) :
	ResourceBase(persistent), texture(texture), hasRTV((rtvFormat != FMT::UNKNOWN)), hasUAV((uavFormat != FMT::UNKNOWN))
{
	//创建SRV、UAV
	{
		setNumCBVSRVUAVDescriptors(1 + texture->getMipLevels() + static_cast<uint32_t>(hasUAV) * texture->getMipLevels());

		D3D12Core::DescriptorHandle descriptorHandle = allocCBVSRVUAVDescriptors();

		//创建访问所有mipslice的SRV
		{
			allSRVIndex = descriptorHandle.getCurrentIndex();

			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

			if (isTextureCube)
			{
				const uint32_t cubeNum = texture->getArraySize() / 6;

				if (cubeNum > 1)
				{
					desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
					desc.Format = srvFormat;
					desc.TextureCubeArray.First2DArrayFace = 0;
					desc.TextureCubeArray.MipLevels = texture->getMipLevels();
					desc.TextureCubeArray.MostDetailedMip = 0;
					desc.TextureCubeArray.ResourceMinLODClamp = 0.f;
					desc.TextureCubeArray.NumCubes = cubeNum;
				}
				else
				{
					desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					desc.Format = srvFormat;
					desc.TextureCube.MipLevels = texture->getMipLevels();
					desc.TextureCube.MostDetailedMip = 0;
					desc.TextureCube.ResourceMinLODClamp = 0.f;
				}
			}
			else
			{
				if (texture->getArraySize() > 1)
				{
					desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					desc.Format = srvFormat;
					desc.Texture2DArray.ArraySize = texture->getArraySize();
					desc.Texture2DArray.FirstArraySlice = 0;
					desc.Texture2DArray.MipLevels = texture->getMipLevels();
					desc.Texture2DArray.MostDetailedMip = 0;
					desc.Texture2DArray.PlaneSlice = 0;
					desc.Texture2DArray.ResourceMinLODClamp = 0.f;
				}
				else
				{
					desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					desc.Format = srvFormat;
					desc.Texture2D.MipLevels = texture->getMipLevels();
					desc.Texture2D.MostDetailedMip = 0;
					desc.Texture2D.PlaneSlice = 0;
					desc.Texture2D.ResourceMinLODClamp = 0.f;
				}
			}

			GraphicsDevice::get()->CreateShaderResourceView(texture->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

			descriptorHandle.move();
		}

		//创建访问各个mipslice的SRV
		{
			srvMipIndices.resize(texture->getMipLevels());

			srvMipGPUHandles.resize(texture->getMipLevels());

			for (uint32_t i = 0; i < texture->getMipLevels(); i++)
			{
				srvMipIndices[i] = descriptorHandle.getCurrentIndex();

				srvMipGPUHandles[i] = descriptorHandle.getCurrentGPUHandle();

				D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

				if (isTextureCube)
				{
					const uint32_t cubeNum = texture->getArraySize() / 6;

					if (cubeNum > 1)
					{
						desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						desc.Format = srvFormat;
						desc.TextureCubeArray.First2DArrayFace = 0;
						desc.TextureCubeArray.MipLevels = 1;
						desc.TextureCubeArray.MostDetailedMip = i;
						desc.TextureCubeArray.ResourceMinLODClamp = 0.f;
						desc.TextureCubeArray.NumCubes = cubeNum;
					}
					else
					{
						desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						desc.Format = srvFormat;
						desc.TextureCube.MipLevels = 1;
						desc.TextureCube.MostDetailedMip = i;
						desc.TextureCube.ResourceMinLODClamp = 0.f;
					}
				}
				else
				{
					if (texture->getArraySize() > 1)
					{
						desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
						desc.Format = srvFormat;
						desc.Texture2DArray.ArraySize = texture->getArraySize();
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
						desc.Format = srvFormat;
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

		if (hasUAV)
		{
			D3D12Core::DescriptorHandle nonShaderVisibleHandle;

			if (persistent)
			{
				nonShaderVisibleHandle = GlobalDescriptorHeap::getStagingResourceHeap()->allocStaticDescriptor(texture->getMipLevels());
			}

			uavMipIndices.resize(texture->getMipLevels());

			viewGPUHandles.resize(texture->getMipLevels());

			viewCPUHandles.resize(texture->getMipLevels());

			for (uint32_t i = 0; i < texture->getMipLevels(); i++)
			{
				uavMipIndices[i] = descriptorHandle.getCurrentIndex();

				D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};

				if (texture->getArraySize() > 1)
				{
					desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					desc.Format = uavFormat;
					desc.Texture2DArray.ArraySize = texture->getArraySize();
					desc.Texture2DArray.FirstArraySlice = 0;
					desc.Texture2DArray.MipSlice = i;
					desc.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					desc.Format = uavFormat;
					desc.Texture2D.MipSlice = i;
					desc.Texture2D.PlaneSlice = 0;
				}

				GraphicsDevice::get()->CreateUnorderedAccessView(texture->getResource(), nullptr, &desc, descriptorHandle.getCurrentCPUHandle());

				if (persistent)
				{
					//同一时间在非着色器可见的资源描述符堆创建UAV
					GraphicsDevice::get()->CreateUnorderedAccessView(texture->getResource(), nullptr, &desc, nonShaderVisibleHandle.getCurrentCPUHandle());

					viewGPUHandles[i] = descriptorHandle.getCurrentGPUHandle();

					viewCPUHandles[i] = nonShaderVisibleHandle.getCurrentCPUHandle();

					nonShaderVisibleHandle.move();
				}
				else
				{
					viewCPUHandles[i] = descriptorHandle.getCurrentCPUHandle();
				}

				descriptorHandle.move();
			}
		}
	}

	//创建RTV
	if (hasRTV)
	{
		D3D12Core::DescriptorHandle descriptorHandle;

		if (persistent)
		{
			descriptorHandle = GlobalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(texture->getMipLevels());
		}
		else
		{
			descriptorHandle = GlobalDescriptorHeap::getRenderTargetHeap()->allocDynamicDescriptor(texture->getMipLevels());
		}

		rtvMipHandles.resize(texture->getMipLevels());

		for (uint32_t i = 0; i < texture->getMipLevels(); i++)
		{
			rtvMipHandles[i] = descriptorHandle.getCurrentCPUHandle();

			D3D12_RENDER_TARGET_VIEW_DESC desc = {};

			if (texture->getArraySize() > 1)
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				desc.Format = rtvFormat;
				desc.Texture2DArray.ArraySize = texture->getArraySize();
				desc.Texture2DArray.FirstArraySlice = 0;
				desc.Texture2DArray.MipSlice = i;
				desc.Texture2DArray.PlaneSlice = 0;
			}
			else
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				desc.Format = rtvFormat;
				desc.Texture2D.MipSlice = i;
				desc.Texture2D.PlaneSlice = 0;
			}

			GraphicsDevice::get()->CreateRenderTargetView(texture->getResource(), &desc, descriptorHandle.getCurrentCPUHandle());

			descriptorHandle.move();
		}
	}
}

Gear::Core::Resource::TextureRenderView::TextureRenderView(const TextureRenderView& trv) :
	ResourceBase(trv.persistent),
	hasRTV(trv.hasRTV),
	hasUAV(trv.hasUAV),
	allSRVIndex(trv.allSRVIndex),
	srvMipIndices(trv.srvMipIndices),
	uavMipIndices(trv.uavMipIndices),
	rtvMipHandles(trv.rtvMipHandles),
	viewGPUHandles(trv.viewGPUHandles),
	viewCPUHandles(trv.viewCPUHandles),
	texture(new D3D12Resource::Texture(*trv.texture))
{
}

Gear::Core::Resource::TextureRenderView::~TextureRenderView()
{
	if (texture)
	{
		delete texture;
	}
}

Gear::Core::Resource::D3D12Resource::ShaderResourceDesc Gear::Core::Resource::TextureRenderView::getAllSRVIndex() const
{
	D3D12Resource::ShaderResourceDesc desc = {};
	desc.type = D3D12Resource::ShaderResourceDesc::TEXTURE;
	desc.state = D3D12Resource::ShaderResourceDesc::SRV;
	desc.resourceIndex = allSRVIndex;
	desc.textureDesc.texture = texture;
	desc.textureDesc.mipSlice = D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS;

	return desc;
}

Gear::Core::Resource::D3D12Resource::ShaderResourceDesc Gear::Core::Resource::TextureRenderView::getSRVMipIndex(const uint32_t mipSlice) const
{
	D3D12Resource::ShaderResourceDesc desc = {};
	desc.type = D3D12Resource::ShaderResourceDesc::TEXTURE;
	desc.state = D3D12Resource::ShaderResourceDesc::SRV;
	desc.resourceIndex = srvMipIndices[mipSlice];
	desc.textureDesc.texture = texture;
	desc.textureDesc.mipSlice = mipSlice;

	return desc;
}

D3D12_GPU_DESCRIPTOR_HANDLE Gear::Core::Resource::TextureRenderView::getSRVMipGPUHandle(const uint32_t mipSlice) const
{
	return srvMipGPUHandles[mipSlice];
}

Gear::Core::Resource::D3D12Resource::ShaderResourceDesc Gear::Core::Resource::TextureRenderView::getUAVMipIndex(const uint32_t mipSlice) const
{
	D3D12Resource::ShaderResourceDesc desc = {};
	desc.type = D3D12Resource::ShaderResourceDesc::TEXTURE;
	desc.state = D3D12Resource::ShaderResourceDesc::UAV;
	desc.resourceIndex = uavMipIndices[mipSlice];
	desc.textureDesc.texture = texture;
	desc.textureDesc.mipSlice = mipSlice;

	return desc;
}

Gear::Core::Resource::D3D12Resource::RenderTargetDesc Gear::Core::Resource::TextureRenderView::getRTVMipHandle(const uint32_t mipSlice) const
{
	D3D12Resource::RenderTargetDesc desc = {};
	desc.texture = texture;
	desc.mipSlice = mipSlice;
	desc.rtvHandle = rtvMipHandles[mipSlice];

	return desc;
}

Gear::Core::Resource::D3D12Resource::ClearUAVDesc Gear::Core::Resource::TextureRenderView::getClearUAVDesc(const uint32_t mipSlice) const
{
	D3D12Resource::ClearUAVDesc desc = {};
	desc.type = D3D12Resource::ClearUAVDesc::TEXTURE;
	desc.textureDesc.texture = texture;
	desc.textureDesc.mipSlice = mipSlice;
	desc.viewGPUHandle = viewGPUHandles[mipSlice];
	desc.viewCPUHandle = viewCPUHandles[mipSlice];

	return desc;
}

Gear::Core::Resource::D3D12Resource::Texture* Gear::Core::Resource::TextureRenderView::getTexture() const
{
	return texture;
}

void Gear::Core::Resource::TextureRenderView::copyDescriptors()
{
	D3D12Core::DescriptorHandle shaderVisibleHandle = copyToResourceHeap();

	allSRVIndex = shaderVisibleHandle.getCurrentIndex();

	shaderVisibleHandle.move();

	for (uint32_t i = 0; i < texture->getMipLevels(); i++)
	{
		srvMipIndices[i] = shaderVisibleHandle.getCurrentIndex();

		shaderVisibleHandle.move();
	}

	if (hasUAV)
	{
		for (uint32_t i = 0; i < texture->getMipLevels(); i++)
		{
			uavMipIndices[i] = shaderVisibleHandle.getCurrentIndex();

			viewGPUHandles[i] = shaderVisibleHandle.getCurrentGPUHandle();

			shaderVisibleHandle.move();
		}
	}
}

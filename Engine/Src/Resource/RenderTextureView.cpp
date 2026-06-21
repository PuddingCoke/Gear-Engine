#include<Gear/Resource/RenderTextureView.h>

namespace Gear::Resource
{
	RenderTextureView::RenderTextureView(D3D12Resource::TexturePtr texturePtr, const bool isTextureCube, const bool persistent, const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat) :
		ResourceBase(persistent), rtvFormat(rtvFormat), uavFormat(uavFormat),
		allSRVIndex(makeShared<uint32_t>()),
		srvMipIndices(makeShared<std::vector<uint32_t>>()),
		srvMipGPUHandles(makeShared<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>>()),
		uavMipIndices(makeShared<std::vector<uint32_t>>()),
		rtvMipHandles(makeShared<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>>()),
		viewGPUHandles(makeShared<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>>()),
		viewCPUHandles(makeShared<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>>()),
		texture(std::move(texturePtr))
	{
		//创建SRV、UAV
		{
			setNumCBVSRVUAVDescriptors(1 + texture->getMipLevels() + static_cast<uint32_t>(uavFormat != FMT::UNKNOWN) * texture->getMipLevels());

			D3D12Core::DescriptorHandle descriptorHandle = allocCBVSRVUAVDescriptors();

			//创建访问所有mipslice的SRV
			{
				*allSRVIndex = descriptorHandle.getCurrentIndex();

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
				(*srvMipIndices).resize(texture->getMipLevels());

				(*srvMipGPUHandles).resize(texture->getMipLevels());

				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					(*srvMipIndices)[i] = descriptorHandle.getCurrentIndex();

					(*srvMipGPUHandles)[i] = descriptorHandle.getCurrentGPUHandle();

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

			if (uavFormat != FMT::UNKNOWN)
			{
				D3D12Core::DescriptorHandle nonShaderVisibleHandle;

				if (persistent)
				{
					nonShaderVisibleHandle = LocalDescriptorHeap::getStagingResourceHeap()->allocStaticDescriptor(texture->getMipLevels());
				}

				(*uavMipIndices).resize(texture->getMipLevels());

				(*viewGPUHandles).resize(texture->getMipLevels());

				(*viewCPUHandles).resize(texture->getMipLevels());

				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					(*uavMipIndices)[i] = descriptorHandle.getCurrentIndex();

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

						(*viewGPUHandles)[i] = descriptorHandle.getCurrentGPUHandle();

						(*viewCPUHandles)[i] = nonShaderVisibleHandle.getCurrentCPUHandle();

						nonShaderVisibleHandle.move();
					}
					else
					{
						(*viewCPUHandles)[i] = descriptorHandle.getCurrentCPUHandle();
					}

					descriptorHandle.move();
				}
			}
		}

		//创建RTV
		if (rtvFormat != FMT::UNKNOWN)
		{
			D3D12Core::DescriptorHandle descriptorHandle;

			if (persistent)
			{
				descriptorHandle = LocalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(texture->getMipLevels());
			}
			else
			{
				descriptorHandle = LocalDescriptorHeap::getRenderTargetHeap()->allocDynamicDescriptor(texture->getMipLevels());
			}

			(*rtvMipHandles).resize(texture->getMipLevels());

			for (uint32_t i = 0; i < texture->getMipLevels(); i++)
			{
				(*rtvMipHandles)[i] = descriptorHandle.getCurrentCPUHandle();

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

	RenderTextureView::RenderTextureView(const RenderTextureView& rtv) :
		ResourceBase(rtv),
		rtvFormat(rtv.rtvFormat),
		uavFormat(rtv.uavFormat),
		allSRVIndex(rtv.allSRVIndex),
		srvMipIndices(rtv.srvMipIndices),
		srvMipGPUHandles(rtv.srvMipGPUHandles),
		uavMipIndices(rtv.uavMipIndices),
		rtvMipHandles(rtv.rtvMipHandles),
		viewGPUHandles(rtv.viewGPUHandles),
		viewCPUHandles(rtv.viewCPUHandles),
		texture(rtv.texture ? makeUnique<D3D12Resource::Texture>(*rtv.texture) : nullptr)
	{
	}

	RenderTextureView::~RenderTextureView()
	{
	}

	ShaderResourceDesc RenderTextureView::getAllSRVIndex() const
	{
		const ShaderResourceDesc desc = {
		.type = ShaderResourceDesc::TEXTURE,
		.state = ShaderResourceDesc::SRV,
		.resourceIndex = allSRVIndex.get(),
		.textureDesc = {
				.texture = texture.get(),
				.mipSlice = D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS
		} };

		return desc;
	}

	ShaderResourceDesc RenderTextureView::getSRVMipIndex(const uint32_t mipSlice) const
	{
		const ShaderResourceDesc desc = {
		.type = ShaderResourceDesc::TEXTURE,
		.state = ShaderResourceDesc::SRV,
		.resourceIndex = &(*srvMipIndices)[mipSlice],
		.textureDesc = {
				.texture = texture.get(),
				.mipSlice = mipSlice
		} };

		return desc;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE RenderTextureView::getSRVMipGPUHandle(const uint32_t mipSlice) const
	{
		return (*srvMipGPUHandles)[mipSlice];
	}

	ShaderResourceDesc RenderTextureView::getUAVMipIndex(const uint32_t mipSlice) const
	{
		const ShaderResourceDesc desc = {
		.type = ShaderResourceDesc::TEXTURE,
		.state = ShaderResourceDesc::UAV,
		.resourceIndex = &(*uavMipIndices)[mipSlice],
		.textureDesc = {
				.texture = texture.get(),
				.mipSlice = mipSlice
		} };

		return desc;
	}

	RenderTargetDesc RenderTextureView::getRTVMipHandle(const uint32_t mipSlice) const
	{
		const RenderTargetDesc desc = {
		.texture = texture.get(),
		.mipSlice = mipSlice,
		.rtvHandle = (*rtvMipHandles)[mipSlice],
		.rtvFormat = getRTVFormat()
		};

		return desc;
	}

	UAVClearDesc RenderTextureView::getUAVMipClearDesc(const uint32_t mipSlice) const
	{
		const UAVClearDesc desc = {
		.type = UAVClearDesc::TEXTURE,
		.textureDesc = {
				.texture = texture.get(),
				.mipSlice = mipSlice
		},
		.viewGPUHandle = (*viewGPUHandles)[mipSlice],
		.viewCPUHandle = (*viewCPUHandles)[mipSlice]
		};

		return desc;
	}

	D3D12Resource::Texture* RenderTextureView::getTexture() const
	{
		return texture.get();
	}

	bool RenderTextureView::copyDescriptors()
	{
		D3D12Core::DescriptorHandle shaderVisibleHandle;

		const bool copied = copyToResourceHeap(shaderVisibleHandle);

		if (copied)
		{
			*allSRVIndex = shaderVisibleHandle.getCurrentIndex();

			shaderVisibleHandle.move();

			for (uint32_t i = 0; i < texture->getMipLevels(); i++)
			{
				(*srvMipIndices)[i] = shaderVisibleHandle.getCurrentIndex();

				shaderVisibleHandle.move();
			}

			if (uavFormat != FMT::UNKNOWN)
			{
				for (uint32_t i = 0; i < texture->getMipLevels(); i++)
				{
					(*uavMipIndices)[i] = shaderVisibleHandle.getCurrentIndex();

					(*viewGPUHandles)[i] = shaderVisibleHandle.getCurrentGPUHandle();

					shaderVisibleHandle.move();
				}
			}
		}

		return copied;
	}

	DXGI_FORMAT RenderTextureView::getRTVFormat() const
	{
		return rtvFormat;
	}

	DXGI_FORMAT RenderTextureView::getUAVFormat() const
	{
		return uavFormat;
	}

	DirectX::XMUINT3 RenderTextureView::get3Dimension(const uint32_t mipSlice) const
	{
		const DirectX::XMUINT2 dimension = get2Dimension(mipSlice);

		return DirectX::XMUINT3{ dimension.x,dimension.y,1u };
	}

	DirectX::XMUINT2 RenderTextureView::get2Dimension(const uint32_t mipSlice) const
	{
#ifdef _DEBUG
		if (mipSlice >= texture->getMipLevels())
		{
			LOGERROR(L"纹理没有这么多的mipLevel");
		}
#endif // _DEBUG

		return DirectX::XMUINT2{ texture->getWidth() >> mipSlice,texture->getHeight() >> mipSlice };
	}

}
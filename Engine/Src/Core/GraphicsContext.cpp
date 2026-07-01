#include<Gear/Core/GraphicsContext.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/TOPOLOGY.h>

#include<Gear/Utils/Math.h>

namespace Gear::Core
{
	static constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE getPrimitiveTopologyType(const D3D_PRIMITIVE_TOPOLOGY topology)
	{
		switch (topology)
		{
		case TOPOLOGY::POINTLIST:
			return TOPOLOGY::TYPE::POINT;
		case TOPOLOGY::LINELIST:
		case TOPOLOGY::LINESTRIP:
		case TOPOLOGY::LINELISTADJ:
		case TOPOLOGY::LINESTRIPADJ:
			return TOPOLOGY::TYPE::LINE;
		case TOPOLOGY::TRIANGLELIST:
		case TOPOLOGY::TRIANGLESTRIP:
		case TOPOLOGY::TRIANGLEFAN:
		case TOPOLOGY::TRIANGLELISTADJ:
		case TOPOLOGY::TRIANGLESTRIPADJ:
			return TOPOLOGY::TYPE::TRIANGLE;
		case TOPOLOGY::PATCH1CONTROL:
		case TOPOLOGY::PATCH2CONTROL:
		case TOPOLOGY::PATCH3CONTROL:
		case TOPOLOGY::PATCH4CONTROL:
		case TOPOLOGY::PATCH5CONTROL:
		case TOPOLOGY::PATCH6CONTROL:
		case TOPOLOGY::PATCH7CONTROL:
		case TOPOLOGY::PATCH8CONTROL:
		case TOPOLOGY::PATCH9CONTROL:
		case TOPOLOGY::PATCH10CONTROL:
		case TOPOLOGY::PATCH11CONTROL:
		case TOPOLOGY::PATCH12CONTROL:
		case TOPOLOGY::PATCH13CONTROL:
		case TOPOLOGY::PATCH14CONTROL:
		case TOPOLOGY::PATCH15CONTROL:
		case TOPOLOGY::PATCH16CONTROL:
		case TOPOLOGY::PATCH17CONTROL:
		case TOPOLOGY::PATCH18CONTROL:
		case TOPOLOGY::PATCH19CONTROL:
		case TOPOLOGY::PATCH20CONTROL:
		case TOPOLOGY::PATCH21CONTROL:
		case TOPOLOGY::PATCH22CONTROL:
		case TOPOLOGY::PATCH23CONTROL:
		case TOPOLOGY::PATCH24CONTROL:
		case TOPOLOGY::PATCH25CONTROL:
		case TOPOLOGY::PATCH26CONTROL:
		case TOPOLOGY::PATCH27CONTROL:
		case TOPOLOGY::PATCH28CONTROL:
		case TOPOLOGY::PATCH29CONTROL:
		case TOPOLOGY::PATCH30CONTROL:
		case TOPOLOGY::PATCH31CONTROL:
		case TOPOLOGY::PATCH32CONTROL:
			return TOPOLOGY::TYPE::PATCH;
		default:
			return TOPOLOGY::TYPE::UNDEFINED;
		}
	}

	static constexpr uint64_t primitiveTopologyTypeTableLength = 128;

	static constexpr std::array<D3D12_PRIMITIVE_TOPOLOGY_TYPE, primitiveTopologyTypeTableLength> primitiveTopologyTypeTable = []() constexpr
		{
			std::array<D3D12_PRIMITIVE_TOPOLOGY_TYPE, primitiveTopologyTypeTableLength> table;

			for (uint32_t i = 0; i < primitiveTopologyTypeTableLength; i++)
			{
				table[i] = getPrimitiveTopologyType(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(i));
			}

			return table;
		}();

	GraphicsContext::GraphicsContext() :
		commandList(makeUnique<D3D12Core::GraphicsCommandList>()),
		vp{ 0.f,0.f,0.f,0.f,0.f,1.f },
		rt{ 0,0,0,0 }
	{
		resetTrackedStates();
	}

	GraphicsContext::~GraphicsContext()
	{
	}

	void GraphicsContext::updateBuffer(Resource::BufferView& bufferView, const void* const data, const uint32_t size) const
	{
		const Resource::BufferView::UpdateStruct updateStruct = bufferView.getUpdateStruct(data, size);

		commandList->copyBufferRegion(updateStruct.buffer, 0, updateStruct.uploadHeap, 0, size);
	}

	void GraphicsContext::updateBuffer(Resource::DefaultCBuffer& defaultCBuffer, const void* const data, const uint32_t size) const
	{
		const Resource::DefaultCBuffer::UpdateStruct updateStruct = defaultCBuffer.getUpdateStruct(data, size);

		commandList->copyBufferRegion(updateStruct.buffer, 0, updateStruct.uploadHeap, 0, size);
	}

	void GraphicsContext::setGlobalConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		if (&immutableCBuffer != userGlobalCBuffer)
		{
			userGlobalCBuffer = &immutableCBuffer;

			D3D12Resource::Buffer* const buffer = userGlobalCBuffer->getBuffer();

			commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

			commandList->flushResourceBarriers();
		}
	}

	void GraphicsContext::setVSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::VERTEX, numValues, offset);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(getGraphicsRootSignature()->getVSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setHSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::HULL, numValues, offset);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(getGraphicsRootSignature()->getHSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setDSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::DOMAIN, numValues, offset);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(getGraphicsRootSignature()->getDSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setGSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::GEOMETRY, numValues, offset);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(getGraphicsRootSignature()->getGSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setPSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::PIXEL, numValues, offset);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(getGraphicsRootSignature()->getPSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setCSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::COMPUTE, numValues, offset);
#endif // _DEBUG

		commandList->setComputeRootConstants(getComputeRootSignature()->getCSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setVSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ getGraphicsRootSignature()->getVSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setHSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ getGraphicsRootSignature()->getHSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setDSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ getGraphicsRootSignature()->getDSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setGSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ getGraphicsRootSignature()->getGSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setPSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ getGraphicsRootSignature()->getPSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setCSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ getComputeRootSignature()->getCSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setPipelineState(D3D12Core::ComputeState& computeState)
	{
		this->computeState = &computeState;

		setPipelineState(computeState.getPipelineState());

		setComputeRootSignature(computeState.getRootSignature());
	}

	void GraphicsContext::setPipelineState(D3D12Core::GraphicsState& graphicsState)
	{
		this->graphicsState = &graphicsState;

		setGraphicsRootSignature(graphicsState.getRootSignature());
	}

	void GraphicsContext::setRenderTargets(const Resource::DepthStencilDesc& depthStencil)
	{
		setResourceState(depthStencil);

		depthStencilClearDesc.setHandle(depthStencil.dsvHandle);

		switchGraphicsPipelineState = true;

		transientNumRTV = 0;

		transientDSVFormat = depthStencil.dsvFormat;

		commandList->setRenderTargets(0, nullptr, FALSE, &depthStencil.dsvHandle);
	}

	void GraphicsContext::setDefRenderTarget()
	{
		commandList->setDefRenderTarget();

		switchGraphicsPipelineState = true;

		transientNumRTV = 1;

		transientRTVFormats[0] = Graphics::backBufferFormat;

		transientDSVFormat = FMT::UNKNOWN;
	}

	void GraphicsContext::clearDefRenderTarget(const float clearValue[4]) const
	{
		commandList->clearRenderTarget(Graphics::getBackBufferHandle(), clearValue, 0, nullptr);
	}

	void GraphicsContext::clearRenderTarget(const Resource::RenderTargetDesc& desc, const float clearValue[4])
	{
		renderTargetClearDescs.push(RenderTargetClearDesc{ desc.rtvHandle,{clearValue[0],clearValue[1],clearValue[2],clearValue[3]} });
	}

	void GraphicsContext::clearRenderTargetInstant(const Resource::RenderTargetDesc& desc, const float clearValue[4])
	{
		clearRenderTarget(desc, clearValue);

		setResourceState(desc);

		transitionResources();

		flushRenderTargetClearDescs();
	}

	void GraphicsContext::clearDepthStencil(const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil)
	{
		depthStencilClearDesc.setClearData(flags, depth, stencil);
	}

	void GraphicsContext::clearDepthStencilInstant(const Resource::DepthStencilDesc& desc, const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil)
	{
		setResourceState(desc);

		transitionResources();

		depthStencilClearDesc.setHandle(desc.dsvHandle);

		depthStencilClearDesc.setClearData(flags, depth, stencil);

		resetDepthStencilClearDesc();
	}

	void GraphicsContext::setIndexBuffer(const Resource::IndexBufferDesc& indexBuffer)
	{
		setResourceState(indexBuffer);

		commandList->setIndexBuffer(&indexBuffer.ibv);
	}

	void GraphicsContext::setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology)
	{
		if (topology != primitiveTopology)
		{
			primitiveTopology = topology;

			transientPrimitiveTopologyType = primitiveTopologyTypeTable[topology];

			commandList->setPrimitiveTopology(primitiveTopology);
		}
	}

	void GraphicsContext::setViewport(const float width, const float height)
	{
		vp.Width = width;
		vp.Height = height;

		commandList->setViewports(1, &vp);
	}

	void GraphicsContext::setViewport(const uint32_t width, const uint32_t height)
	{
		setViewport(static_cast<float>(width), static_cast<float>(height));
	}

	void GraphicsContext::setViewport(const DirectX::XMUINT2 dimension)
	{
		setViewport(dimension.x, dimension.y);
	}

	void GraphicsContext::setScissorRect(const int32_t left, const int32_t top, const int32_t right, const int32_t bottom)
	{
		rt.left = left;
		rt.top = top;
		rt.right = right;
		rt.bottom = bottom;

		commandList->setScissorRects(1, &rt);
	}

	void GraphicsContext::setScissorRect(const float left, const float top, const float right, const float bottom)
	{
		setScissorRect(static_cast<int32_t>(left), static_cast<int32_t>(top), static_cast<int32_t>(right), static_cast<int32_t>(bottom));
	}

	void GraphicsContext::setViewportSimple(const float width, const float height)
	{
		setViewport(width, height);

		setScissorRect(0.f, 0.f, width, height);
	}

	void GraphicsContext::setViewportSimple(const uint32_t width, const uint32_t height)
	{
		setViewport(width, height);

		setScissorRect(0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height));
	}

	void GraphicsContext::setViewportSimple(const DirectX::XMUINT2 dimension)
	{
		setViewport(dimension);

		setScissorRect(0, 0, static_cast<int32_t>(dimension.x), static_cast<int32_t>(dimension.y));
	}

	void GraphicsContext::clearUnorderedAccess(const Resource::UAVClearDesc& desc, const float values[4])
	{
		D3D12Resource::D3D12ResourceBase* resource = setResourceState(desc);

		commandList->flushTransitionResources();

		commandList->pushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource()));

		commandList->flushResourceBarriers();

		commandList->clearUnorderedAccessFloat(desc.viewGPUHandle, desc.viewCPUHandle, resource->getResource(), values, 0, nullptr);

		commandList->pushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource()));

		commandList->flushResourceBarriers();
	}

	void GraphicsContext::clearUnorderedAccess(const Resource::UAVClearDesc& desc, const uint32_t values[4])
	{
		D3D12Resource::D3D12ResourceBase* resource = setResourceState(desc);

		commandList->flushTransitionResources();

		commandList->pushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource()));

		commandList->flushResourceBarriers();

		commandList->clearUnorderedAccessUint(desc.viewGPUHandle, desc.viewCPUHandle, resource->getResource(), values, 0, nullptr);

		commandList->pushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource()));

		commandList->flushResourceBarriers();
	}

	void GraphicsContext::draw(const uint32_t vertexCountPerInstance, const uint32_t instanceCount, const uint32_t startVertexLocation, const uint32_t startInstanceLocation)
	{
		transitionResources();

		flushRootConstantBufferDescs(true);

		flushRenderTargetClearDescs();

		resetDepthStencilClearDesc();

		//只有调用setRenderTargets后才需要检测是否需要并绑定新的管线状态
		//setPipelineState调用后必定跟随至少一次setRenderTargets
		//所以能保证切换图形管线状态时setPipelineState被调用
		if (switchGraphicsPipelineState)
		{
			graphicsState->updatePipelineState(transientRTVFormats.data(), transientNumRTV, transientDSVFormat, transientPrimitiveTopologyType);

			setPipelineState(graphicsState->getPipelineState());

			switchGraphicsPipelineState = false;
		}

		commandList->drawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	}

	void GraphicsContext::drawQuad()
	{
		draw(3, 1, 0, 0);
	}

	void GraphicsContext::drawIndexed(const uint32_t indexCountPerInstance, const uint32_t instanceCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation, const uint32_t startInstanceLocation)
	{
		transitionResources();

		flushRootConstantBufferDescs(true);

		flushRenderTargetClearDescs();

		resetDepthStencilClearDesc();

		if (switchGraphicsPipelineState)
		{
			graphicsState->updatePipelineState(transientRTVFormats.data(), transientNumRTV, transientDSVFormat, transientPrimitiveTopologyType);

			setPipelineState(graphicsState->getPipelineState());

			switchGraphicsPipelineState = false;
		}

		commandList->drawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	void GraphicsContext::dispatchGrp(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ)
	{
		transitionResources();

		flushRootConstantBufferDescs(false);

		commandList->dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	void GraphicsContext::dispatchDim(const uint32_t dispatchThreadCountX, const uint32_t dispatchThreadCountY, const uint32_t dispatchThreadCountZ)
	{
		const DirectX::XMUINT3 groupDimension = computeState->getPipelineStateData().computeData.groupDimension;

		dispatchGrp(
			Utils::Math::ceil(dispatchThreadCountX, groupDimension.x),
			Utils::Math::ceil(dispatchThreadCountY, groupDimension.y),
			Utils::Math::ceil(dispatchThreadCountZ, groupDimension.z)
		);
	}

	void GraphicsContext::dispatchDim(const DirectX::XMUINT3 dispatchThreadCount)
	{
		dispatchDim(dispatchThreadCount.x, dispatchThreadCount.y, dispatchThreadCount.z);
	}

	void GraphicsContext::begin()
	{
		commandList->open();

		commandList->setDescriptorHeap(GlobalDescriptorHeap::getResourceHeap()->get(), GlobalDescriptorHeap::getSamplerHeap()->get());

		resetTrackedStates();
	}

	void GraphicsContext::resetTrackedStates()
	{
		resetPipelineState();

		resetTrackedGraphicsStates();

		resetTrackedComputeStates();

		resetUserGlobalCBuffer();
	}

	void GraphicsContext::resetPipelineState()
	{
		currentPipelineState = nullptr;
	}

	void GraphicsContext::resetTrackedGraphicsStates()
	{
		resetGraphicsRootSignature();

		resetPrimitiveTopology();
	}

	void GraphicsContext::resetTrackedComputeStates()
	{
		resetComputeRootSignature();
	}

	D3D12Core::GraphicsCommandList* GraphicsContext::getCommandList() const
	{
		return commandList.get();
	}

	void GraphicsContext::transitionResources()
	{
		commandList->flushResourceBarriers();
	}

	void GraphicsContext::setPipelineState(ID3D12PipelineState* const pipelineState)
	{
		if (pipelineState != currentPipelineState)
		{
			currentPipelineState = pipelineState;

			commandList->setPipelineState(currentPipelineState);
		}
	}

	void GraphicsContext::setShaderResources(const std::vector<Resource::ShaderResourceDesc>& descs, const uint32_t targetSRVState)
	{
		for (const Resource::ShaderResourceDesc& desc : descs)
		{
			setResourceState(desc, targetSRVState);
		}
	}

	void GraphicsContext::pushRootConstantBufferDesc(const RootConstantBufferDesc& desc)
	{
		rootConstantBufferDescs.push(desc);
	}

	void GraphicsContext::flushRootConstantBufferDescs(const bool isGraphicsRootSignature)
	{
		if (rootConstantBufferDescs.size())
		{
			if (isGraphicsRootSignature)
			{
				for (const RootConstantBufferDesc& desc : rootConstantBufferDescs)
				{
					const uint32_t rootParameterIndex = desc.rootParameterIndex;

					const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = desc.gpuAddress;

					commandList->setGraphicsRootConstantBuffer(rootParameterIndex, gpuAddress);
				}
			}
			else
			{
				for (const RootConstantBufferDesc& desc : rootConstantBufferDescs)
				{
					const uint32_t rootParameterIndex = desc.rootParameterIndex;

					const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = desc.gpuAddress;

					commandList->setComputeRootConstantBuffer(rootParameterIndex, gpuAddress);
				}
			}

			rootConstantBufferDescs.clear();
		}
	}

	void GraphicsContext::flushRenderTargetClearDescs()
	{
		if (renderTargetClearDescs.size())
		{
			for (const RenderTargetClearDesc& desc : renderTargetClearDescs)
			{
				const D3D12_CPU_DESCRIPTOR_HANDLE handle = desc.handle;

				const float* const clearValue = desc.clearValue;

				commandList->clearRenderTarget(handle, clearValue, 0, nullptr);
			}

			renderTargetClearDescs.clear();
		}
	}

	void GraphicsContext::resetDepthStencilClearDesc()
	{
		if (depthStencilClearDesc.getFlags())
		{
			commandList->clearDepthStencil(depthStencilClearDesc.getHandle(), depthStencilClearDesc.getFlags(), depthStencilClearDesc.getDepth(), depthStencilClearDesc.getStencil(), 0, nullptr);

			depthStencilClearDesc.reset();
		}
	}

	void GraphicsContext::setGraphicsRootSignature(const D3D12Core::RootSignature* const rootSignature)
	{
		if (rootSignature != graphicsRootSignature)
		{
			graphicsRootSignature = rootSignature;

			commandList->setGraphicsRootSignature(rootSignature->get());

			commandList->setGraphicsRootConstantBuffer(D3D12Core::RootSignature::getEngineGlobalConstantBufferParameterIndex(), Graphics::getEngineGlobalCBuffer()->getGPUAddress());

			if (userGlobalCBuffer)
			{
				commandList->setGraphicsRootConstantBuffer(D3D12Core::RootSignature::getUserGlobalConstantBufferParameterIndex(), userGlobalCBuffer->getGPUAddress());
			}
		}
	}

	void GraphicsContext::setComputeRootSignature(const D3D12Core::RootSignature* const rootSignature)
	{
		if (rootSignature != computeRootSignature)
		{
			computeRootSignature = rootSignature;

			commandList->setComputeRootSignature(rootSignature->get());

			commandList->setComputeRootConstantBuffer(D3D12Core::RootSignature::getEngineGlobalConstantBufferParameterIndex(), Graphics::getEngineGlobalCBuffer()->getGPUAddress());

			if (userGlobalCBuffer)
			{
				commandList->setComputeRootConstantBuffer(D3D12Core::RootSignature::getUserGlobalConstantBufferParameterIndex(), userGlobalCBuffer->getGPUAddress());
			}
		}
	}

	const D3D12Core::RootSignature* GraphicsContext::getGraphicsRootSignature() const
	{
#ifdef _DEBUG
		if (nullptr == graphicsRootSignature)
		{
			LOGERROR(L"没有检测到图形根签名绑定，是否忘记设置管线状态？");
		}
#endif // _DEBUG

		return graphicsRootSignature;
	}

	const D3D12Core::RootSignature* GraphicsContext::getComputeRootSignature() const
	{
#ifdef _DEBUG
		if (nullptr == computeRootSignature)
		{
			LOGERROR(L"没有检测到计算根签名绑定，是否忘记设置管线状态？");
		}
#endif // _DEBUG

		return computeRootSignature;
	}

	void GraphicsContext::setResourceState(const Resource::ShaderResourceDesc& desc, const uint32_t targetSRVState)
	{
		if (desc.type == Resource::ShaderResourceDesc::BUFFER)
		{
			if (desc.state == Resource::ShaderResourceDesc::SRV)
			{
				commandList->trackAndSetResourceState(desc.bufferDesc.buffer, targetSRVState);
			}
			else if (desc.state == Resource::ShaderResourceDesc::UAV)
			{
				commandList->trackAndSetResourceState(desc.bufferDesc.buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				commandList->trackAndSetResourceState(desc.bufferDesc.counterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			}
			else
			{
				commandList->trackAndSetResourceState(desc.bufferDesc.buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			}
		}
		else if (desc.type == Resource::ShaderResourceDesc::TEXTURE)
		{
			if (desc.state == Resource::ShaderResourceDesc::SRV)
			{
				commandList->trackAndSetResourceState(desc.textureDesc.texture, desc.textureDesc.mipSlice, targetSRVState);
			}
			else if (desc.state == Resource::ShaderResourceDesc::UAV)
			{
				commandList->trackAndSetResourceState(desc.textureDesc.texture, desc.textureDesc.mipSlice, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			}
		}
	}

	void GraphicsContext::setResourceState(const Resource::RenderTargetDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.texture, desc.mipSlice, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	void GraphicsContext::setResourceState(const Resource::DepthStencilDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.texture, desc.mipSlice, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	void GraphicsContext::setResourceState(const Resource::VertexBufferDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	void GraphicsContext::setResourceState(const Resource::IndexBufferDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.buffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	D3D12Resource::D3D12ResourceBase* GraphicsContext::setResourceState(const Resource::UAVClearDesc& desc)
	{
		D3D12Resource::D3D12ResourceBase* resource = nullptr;

		if (desc.type == Resource::UAVClearDesc::BUFFER)
		{
			resource = desc.bufferDesc.buffer;

			commandList->trackAndSetResourceState(desc.bufferDesc.buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
		else if (desc.type == Resource::UAVClearDesc::TEXTURE)
		{
			resource = desc.textureDesc.texture;

			commandList->trackAndSetResourceState(desc.textureDesc.texture, desc.textureDesc.mipSlice, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		return resource;
	}

	void GraphicsContext::resetGraphicsRootSignature()
	{
		graphicsRootSignature = nullptr;
	}

	void GraphicsContext::resetPrimitiveTopology()
	{
		primitiveTopology = TOPOLOGY::UNDEFINED;

		transientPrimitiveTopologyType = TOPOLOGY::TYPE::UNDEFINED;
	}

	void GraphicsContext::resetComputeRootSignature()
	{
		computeRootSignature = nullptr;
	}

	void GraphicsContext::resetUserGlobalCBuffer()
	{
		userGlobalCBuffer = nullptr;
	}

	void GraphicsContext::constantsWriteCheck(const D3D12Core::RootSignature::ShaderType shaderType, const uint32_t numWrite, const uint32_t offset) const
	{
		uint32_t numShaderConstants = 0u;

		switch (shaderType)
		{
		case D3D12Core::RootSignature::ShaderType::VERTEX:
		case D3D12Core::RootSignature::ShaderType::HULL:
		case D3D12Core::RootSignature::ShaderType::DOMAIN:
		case D3D12Core::RootSignature::ShaderType::GEOMETRY:
		case D3D12Core::RootSignature::ShaderType::PIXEL:
			numShaderConstants = getGraphicsRootSignature()->getNumShaderConstants(shaderType);
			break;
		case D3D12Core::RootSignature::ShaderType::COMPUTE:
			numShaderConstants = getComputeRootSignature()->getNumShaderConstants(shaderType);
			break;
		default:
			break;
		}

		if (numShaderConstants < numWrite + offset)
		{
			std::wstring shaderName = L"";

			switch (shaderType)
			{
			case D3D12Core::RootSignature::ShaderType::VERTEX:
				shaderName = L"顶点着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::HULL:
				shaderName = L"外壳着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::DOMAIN:
				shaderName = L"域着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::GEOMETRY:
				shaderName = L"几何着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::PIXEL:
				shaderName = L"像素着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::COMPUTE:
				shaderName = L"计算着色器";
				break;
			default:
				break;
			}

			LOGERROR(L"侦测到常量写入终止位置", offset + numWrite - 1u, L"越界", shaderName, L"的常量写入终止位置最高为", numShaderConstants - 1u);
		}
	}

	void GraphicsContext::DepthStencilClearDesc::setHandle(const D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept
	{
		this->handle = handle;
	}

	void GraphicsContext::DepthStencilClearDesc::setClearData(const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil) noexcept
	{
		this->flags = flags;

		this->depth = depth;

		this->stencil = stencil;
	}

	void GraphicsContext::DepthStencilClearDesc::reset() noexcept
	{
		this->flags = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GraphicsContext::DepthStencilClearDesc::getHandle() const noexcept
	{
		return handle;
	}

	D3D12_CLEAR_FLAGS GraphicsContext::DepthStencilClearDesc::getFlags() const noexcept
	{
		return static_cast<D3D12_CLEAR_FLAGS>(flags);
	}

	float GraphicsContext::DepthStencilClearDesc::getDepth() const noexcept
	{
		return depth;
	}

	uint8_t GraphicsContext::DepthStencilClearDesc::getStencil() const noexcept
	{
		return stencil;
	}
}
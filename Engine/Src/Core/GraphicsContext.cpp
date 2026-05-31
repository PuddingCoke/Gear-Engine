#include<Gear/Core/GraphicsContext.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/GlobalRootSignature.h>

Gear::Core::GraphicsContext::GraphicsContext() :
	commandList(new D3D12Core::CommandList(D3D12_COMMAND_LIST_TYPE_DIRECT)),
	vp{ 0.f,0.f,0.f,0.f,0.f,1.f },
	rt{ 0,0,0,0 },
	resourceIndices{}
{
	resetTrackedStates();
}

Gear::Core::GraphicsContext::~GraphicsContext()
{
	if (commandList)
	{
		delete commandList;
	}
}

void Gear::Core::GraphicsContext::updateBuffer(Resource::BufferView& bufferView, const void* const data, const uint32_t size) const
{
	const Resource::BufferView::UpdateStruct updateStruct = bufferView.getUpdateStruct(data, size);

	commandList->copyBufferRegion(updateStruct.buffer, 0, updateStruct.uploadHeap, 0, size);
}

void Gear::Core::GraphicsContext::updateBuffer(Resource::StaticCBuffer& staticCBuffer, const void* const data, const uint32_t size) const
{
	const Resource::StaticCBuffer::UpdateStruct updateStruct = staticCBuffer.getUpdateStruct(data, size);

	commandList->copyBufferRegion(updateStruct.buffer, 0, updateStruct.uploadHeap, 0, size);
}

void Gear::Core::GraphicsContext::setGlobalConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
{
	if (&immutableCBuffer != userDefinedGlobalConstantBuffer)
	{
		userDefinedGlobalConstantBuffer = &immutableCBuffer;

		Resource::D3D12Resource::Buffer* const buffer = userDefinedGlobalConstantBuffer->getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		commandList->transitionResources();
	}
}

void Gear::Core::GraphicsContext::setVSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const
{
	commandList->setGraphicsRootConstants(graphicsRootSignature->getVSConstantsParameterIndex(), numValues, data, offset);
}

void Gear::Core::GraphicsContext::setHSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const
{
	commandList->setGraphicsRootConstants(graphicsRootSignature->getHSConstantsParameterIndex(), numValues, data, offset);
}

void Gear::Core::GraphicsContext::setDSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const
{
	commandList->setGraphicsRootConstants(graphicsRootSignature->getDSConstantsParameterIndex(), numValues, data, offset);
}

void Gear::Core::GraphicsContext::setGSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const
{
	commandList->setGraphicsRootConstants(graphicsRootSignature->getGSConstantsParameterIndex(), numValues, data, offset);
}

void Gear::Core::GraphicsContext::setPSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const
{
	commandList->setGraphicsRootConstants(graphicsRootSignature->getPSConstantsParameterIndex(), numValues, data, offset);
}

void Gear::Core::GraphicsContext::setCSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const
{
	commandList->setComputeRootConstants(computeRootSignature->getCSConstantsParameterIndex(), numValues, data, offset);
}

void Gear::Core::GraphicsContext::setVSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
{
	Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

	commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	pushRootConstantBufferDesc({ graphicsRootSignature->getVSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
}

void Gear::Core::GraphicsContext::setHSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
{
	Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

	commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	pushRootConstantBufferDesc({ graphicsRootSignature->getHSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
}

void Gear::Core::GraphicsContext::setDSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
{
	Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

	commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	pushRootConstantBufferDesc({ graphicsRootSignature->getDSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
}

void Gear::Core::GraphicsContext::setGSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
{
	Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

	commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	pushRootConstantBufferDesc({ graphicsRootSignature->getGSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
}

void Gear::Core::GraphicsContext::setPSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
{
	Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

	commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	pushRootConstantBufferDesc({ graphicsRootSignature->getPSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
}

void Gear::Core::GraphicsContext::setCSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
{
	Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

	commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	pushRootConstantBufferDesc({ computeRootSignature->getCSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
}

void Gear::Core::GraphicsContext::setPipelineState(const D3D12Core::PipelineState& pipelineState)
{
	if (&pipelineState != currentPipelineState)
	{
		currentPipelineState = &pipelineState;

		commandList->setPipelineState(currentPipelineState->get());

		if (currentPipelineState->getPipelineStateType() == D3D12Core::PipelineState::PipelineStateType::GRAPHICS)
		{
			setGraphicsRootSignature(currentPipelineState->getRootSignature());
		}
		else
		{
			setComputeRootSignature(currentPipelineState->getRootSignature());
		}
	}
}

void Gear::Core::GraphicsContext::setRenderTargets(const Resource::D3D12Resource::DepthStencilDesc& depthStencil)
{
#ifdef _DEBUG
	if (nullptr == depthStencil.texture)
	{
		LOGERROR(L"if you want to bind only depth stencil view then texture pointer cannot be nullptr");
	}
#endif // _DEBUG

	transientRTVHandles.clear();

	setResourceState(depthStencil);

	commandList->setRenderTargets(static_cast<uint32_t>(transientRTVHandles.size()), transientRTVHandles.data(), FALSE, &depthStencil.dsvHandle);
}

void Gear::Core::GraphicsContext::setDefRenderTarget() const
{
	commandList->setDefRenderTarget();
}

void Gear::Core::GraphicsContext::clearDefRenderTarget(const float clearValue[4]) const
{
	commandList->clearRenderTarget(Graphics::getBackBufferHandle(), clearValue, 0, nullptr);
}

void Gear::Core::GraphicsContext::clearRenderTarget(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4])
{
	renderTargetClearDescs.emplace_back(RenderTargetClearDesc{ desc.rtvHandle,{clearValue[0],clearValue[1],clearValue[2],clearValue[3]} });
}

void Gear::Core::GraphicsContext::clearDepthStencil(const Resource::D3D12Resource::DepthStencilDesc& desc, const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil)
{
	depthStencilClearDescs.emplace_back(DepthStencilClearDesc{ desc.dsvHandle,flags,depth,stencil });
}

void Gear::Core::GraphicsContext::clearRenderTargetInstant(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4])
{
	clearRenderTarget(desc, clearValue);

	setResourceState(desc);

	transitionResources();

	flushRenderTargetClearDescs();
}

void Gear::Core::GraphicsContext::clearDepthStencilInstant(const Resource::D3D12Resource::DepthStencilDesc& desc, const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil)
{
	clearDepthStencil(desc, flags, depth, stencil);

	setResourceState(desc);

	transitionResources();

	flushDepthStencilClearDescs();
}

void Gear::Core::GraphicsContext::setIndexBuffer(const Resource::D3D12Resource::IndexBufferDesc& indexBuffer)
{
	setResourceState(indexBuffer);

	commandList->setIndexBuffer(&indexBuffer.ibv);
}

void Gear::Core::GraphicsContext::setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology)
{
	if (topology != primitiveTopology)
	{
		primitiveTopology = topology;

		commandList->setPrimitiveTopology(primitiveTopology);
	}
}

void Gear::Core::GraphicsContext::setViewport(const float width, const float height)
{
	vp.Width = width;
	vp.Height = height;

	commandList->setViewports(1, &vp);
}

void Gear::Core::GraphicsContext::setViewport(const uint32_t width, const uint32_t height)
{
	setViewport(static_cast<float>(width), static_cast<float>(height));
}

void Gear::Core::GraphicsContext::setScissorRect(const uint32_t left, const uint32_t top, const uint32_t right, const uint32_t bottom)
{
	rt.left = left;
	rt.top = top;
	rt.right = right;
	rt.bottom = bottom;

	commandList->setScissorRects(1, &rt);
}

void Gear::Core::GraphicsContext::setScissorRect(const float left, const float top, const float right, const float bottom)
{
	setScissorRect(static_cast<uint32_t>(left), static_cast<uint32_t>(top), static_cast<uint32_t>(right), static_cast<uint32_t>(bottom));
}

void Gear::Core::GraphicsContext::setViewportSimple(const float width, const float height)
{
	setViewport(width, height);
	setScissorRect(0, 0, width, height);
}

void Gear::Core::GraphicsContext::setViewportSimple(const uint32_t width, const uint32_t height)
{
	setViewport(width, height);
	setScissorRect(0, 0, width, height);
}

void Gear::Core::GraphicsContext::clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const float values[4])
{
	Resource::D3D12Resource::D3D12ResourceBase* resource = setResourceState(desc);

	transitionResources();

	uavBarrier({ resource });

	commandList->clearUnorderedAccessFloat(desc.viewGPUHandle, desc.viewCPUHandle, resource->getResource(), values, 0, nullptr);

	uavBarrier({ resource });
}

void Gear::Core::GraphicsContext::clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const uint32_t values[4])
{
	Resource::D3D12Resource::D3D12ResourceBase* resource = setResourceState(desc);

	transitionResources();

	uavBarrier({ resource });

	commandList->clearUnorderedAccessUint(desc.viewGPUHandle, desc.viewCPUHandle, resource->getResource(), values, 0, nullptr);

	uavBarrier({ resource });
}

void Gear::Core::GraphicsContext::draw(const uint32_t vertexCountPerInstance, const uint32_t instanceCount, const uint32_t startVertexLocation, const uint32_t startInstanceLocation)
{
	transitionResources();

	flushRootConstantBufferDescs(true);

	flushRenderTargetClearDescs();

	flushDepthStencilClearDescs();

	commandList->drawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void Gear::Core::GraphicsContext::drawIndexed(const uint32_t indexCountPerInstance, const uint32_t instanceCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation, const uint32_t startInstanceLocation)
{
	transitionResources();

	flushRootConstantBufferDescs(true);

	flushRenderTargetClearDescs();

	flushDepthStencilClearDescs();

	commandList->drawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void Gear::Core::GraphicsContext::dispatch(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ)
{
	transitionResources();

	flushRootConstantBufferDescs(false);

	commandList->dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void Gear::Core::GraphicsContext::begin()
{
	commandList->open();

	commandList->setDescriptorHeap(GlobalDescriptorHeap::getResourceHeap()->get(), GlobalDescriptorHeap::getSamplerHeap()->get());

	resetTrackedStates();
}

void Gear::Core::GraphicsContext::resetTrackedStates()
{
	resetPipelineState();

	resetTrackedGraphicsStates();

	resetTrackedComputeStates();

	resetUserDefinedGlobalConstantBuffer();
}

void Gear::Core::GraphicsContext::resetPipelineState()
{
	currentPipelineState = nullptr;
}

void Gear::Core::GraphicsContext::resetTrackedGraphicsStates()
{
	resetGraphicsRootSignature();

	resetPrimitiveTopology();
}

void Gear::Core::GraphicsContext::resetTrackedComputeStates()
{
	resetComputeRootSignature();
}

Gear::Core::D3D12Core::CommandList* Gear::Core::GraphicsContext::getCommandList() const
{
	return commandList;
}

void Gear::Core::GraphicsContext::transitionResources()
{
	commandList->transitionResources();
}

void Gear::Core::GraphicsContext::pushRootConstantBufferDesc(const RootConstantBufferDesc& desc)
{
	rootConstantBufferDescs.emplace_back(desc);
}

void Gear::Core::GraphicsContext::flushRootConstantBufferDescs(const bool isGraphicsRootSignature)
{
	if (rootConstantBufferDescs.size())
	{
		if (isGraphicsRootSignature)
		{
			for (const RootConstantBufferDesc& desc : rootConstantBufferDescs)
			{
				commandList->setGraphicsRootConstantBuffer(desc.rootParameterIndex, desc.gpuAddress);
			}
		}
		else
		{
			for (const RootConstantBufferDesc& desc : rootConstantBufferDescs)
			{
				commandList->setComputeRootConstantBuffer(desc.rootParameterIndex, desc.gpuAddress);
			}
		}

		rootConstantBufferDescs.clear();
	}
}

void Gear::Core::GraphicsContext::flushRenderTargetClearDescs()
{
	if (renderTargetClearDescs.size())
	{
		for (const RenderTargetClearDesc& desc : renderTargetClearDescs)
		{
			commandList->clearRenderTarget(desc.handle, desc.clearValue, 0, nullptr);
		}

		renderTargetClearDescs.clear();
	}
}

void Gear::Core::GraphicsContext::flushDepthStencilClearDescs()
{
	if (depthStencilClearDescs.size())
	{
		for (const DepthStencilClearDesc& desc : depthStencilClearDescs)
		{
			commandList->clearDepthStencil(desc.handle, desc.flags, desc.depth, desc.stencil, 0, nullptr);
		}

		depthStencilClearDescs.clear();
	}
}

void Gear::Core::GraphicsContext::setGraphicsRootSignature(const D3D12Core::RootSignature* const rootSignature)
{
	if (rootSignature != graphicsRootSignature)
	{
		graphicsRootSignature = rootSignature;

		commandList->setGraphicsRootSignature(rootSignature->get());

		commandList->setGraphicsRootConstantBuffer(D3D12Core::RootSignature::getEngineDefinedGlobalConstantBufferParameterIndex(), Graphics::getEngineDefinedGlobalCBuffer()->getGPUAddress());

		if (userDefinedGlobalConstantBuffer)
			commandList->setGraphicsRootConstantBuffer(D3D12Core::RootSignature::getUserDefinedGlobalConstantBufferParameterIndex(), userDefinedGlobalConstantBuffer->getGPUAddress());
	}
}

void Gear::Core::GraphicsContext::setComputeRootSignature(const D3D12Core::RootSignature* const rootSignature)
{
	if (rootSignature != computeRootSignature)
	{
		computeRootSignature = rootSignature;

		commandList->setComputeRootSignature(rootSignature->get());

		commandList->setComputeRootConstantBuffer(D3D12Core::RootSignature::getEngineDefinedGlobalConstantBufferParameterIndex(), Graphics::getEngineDefinedGlobalCBuffer()->getGPUAddress());

		if (userDefinedGlobalConstantBuffer)
			commandList->setComputeRootConstantBuffer(D3D12Core::RootSignature::getUserDefinedGlobalConstantBufferParameterIndex(), userDefinedGlobalConstantBuffer->getGPUAddress());
	}
}

void Gear::Core::GraphicsContext::setResourceState(const Resource::D3D12Resource::ShaderResourceDesc& desc, const uint32_t targetSRVState)
{
	if (desc.type == Resource::D3D12Resource::ShaderResourceDesc::BUFFER)
	{
		if (desc.state == Resource::D3D12Resource::ShaderResourceDesc::SRV)
		{
			commandList->trackAndSetResourceState(desc.bufferDesc.buffer, targetSRVState);
		}
		else if (desc.state == Resource::D3D12Resource::ShaderResourceDesc::UAV)
		{
			commandList->trackAndSetResourceState(desc.bufferDesc.buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			commandList->trackAndSetResourceState(desc.bufferDesc.counterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
		else
		{
			commandList->trackAndSetResourceState(desc.bufferDesc.buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
	}
	else if (desc.type == Resource::D3D12Resource::ShaderResourceDesc::TEXTURE)
	{
		if (desc.state == Resource::D3D12Resource::ShaderResourceDesc::SRV)
		{
			commandList->trackAndSetResourceState(desc.textureDesc.texture, desc.textureDesc.mipSlice, targetSRVState);
		}
		else if (desc.state == Resource::D3D12Resource::ShaderResourceDesc::UAV)
		{
			commandList->trackAndSetResourceState(desc.textureDesc.texture, desc.textureDesc.mipSlice, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}
}

void Gear::Core::GraphicsContext::setResourceState(const Resource::D3D12Resource::RenderTargetDesc& desc)
{
	commandList->trackAndSetResourceState(desc.texture, desc.mipSlice, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void Gear::Core::GraphicsContext::setResourceState(const Resource::D3D12Resource::DepthStencilDesc& desc)
{
	commandList->trackAndSetResourceState(desc.texture, desc.mipSlice, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void Gear::Core::GraphicsContext::setResourceState(const Resource::D3D12Resource::VertexBufferDesc& desc)
{
	commandList->trackAndSetResourceState(desc.buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

void Gear::Core::GraphicsContext::setResourceState(const Resource::D3D12Resource::IndexBufferDesc& desc)
{
	commandList->trackAndSetResourceState(desc.buffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
}

Gear::Core::Resource::D3D12Resource::D3D12ResourceBase* Gear::Core::GraphicsContext::setResourceState(const Resource::D3D12Resource::ClearUAVDesc& desc)
{
	Resource::D3D12Resource::D3D12ResourceBase* resource = nullptr;

	if (desc.type == Resource::D3D12Resource::ClearUAVDesc::BUFFER)
	{
		resource = desc.bufferDesc.buffer;

		commandList->trackAndSetResourceState(desc.bufferDesc.buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else if (desc.type == Resource::D3D12Resource::ClearUAVDesc::TEXTURE)
	{
		resource = desc.textureDesc.texture;

		commandList->trackAndSetResourceState(desc.textureDesc.texture, desc.textureDesc.mipSlice, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	return resource;
}

void Gear::Core::GraphicsContext::resetGraphicsRootSignature()
{
	graphicsRootSignature = nullptr;
}

void Gear::Core::GraphicsContext::resetPrimitiveTopology()
{
	primitiveTopology = TOPOLOGY::UNDEFINED;
}

void Gear::Core::GraphicsContext::resetComputeRootSignature()
{
	computeRootSignature = nullptr;
}

void Gear::Core::GraphicsContext::resetUserDefinedGlobalConstantBuffer()
{
	userDefinedGlobalConstantBuffer = nullptr;
}
#include<Gear/Core/GraphicsContext.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/GlobalRootSignature.h>

namespace Gear::Core
{
	GraphicsContext::GraphicsContext() :
		commandList(makeUnique<D3D12Core::CommandList>(D3D12_COMMAND_LIST_TYPE_DIRECT)),
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

	void GraphicsContext::updateBuffer(Resource::StaticCBuffer& staticCBuffer, const void* const data, const uint32_t size) const
	{
		const Resource::StaticCBuffer::UpdateStruct updateStruct = staticCBuffer.getUpdateStruct(data, size);

		commandList->copyBufferRegion(updateStruct.buffer, 0, updateStruct.uploadHeap, 0, size);
	}

	void GraphicsContext::setGlobalConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		if (&immutableCBuffer != userDefinedGlobalConstantBuffer)
		{
			userDefinedGlobalConstantBuffer = &immutableCBuffer;

			Resource::D3D12Resource::Buffer* const buffer = userDefinedGlobalConstantBuffer->getBuffer();

			commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

			commandList->transitionResources();
		}
	}

	void GraphicsContext::constantsWriteCheck(const D3D12Core::RootSignature::ShaderType shaderType, const uint32_t numWrite) const
	{
		uint32_t numShaderConstants = 0u;

		switch (shaderType)
		{
		case D3D12Core::RootSignature::ShaderType::VERTEX:
		case D3D12Core::RootSignature::ShaderType::HULL:
		case D3D12Core::RootSignature::ShaderType::DOMAIN:
		case D3D12Core::RootSignature::ShaderType::GEOMETRY:
		case D3D12Core::RootSignature::ShaderType::PIXEL:
			numShaderConstants = graphicsRootSignature->getNumShaderConstants(shaderType);
			break;
		case D3D12Core::RootSignature::ShaderType::COMPUTE:
			numShaderConstants = computeRootSignature->getNumShaderConstants(shaderType);
			break;
		default:
			break;
		}

		if (numShaderConstants < numWrite)
		{
			std::wstring errorString = L"";

			switch (shaderType)
			{
			case D3D12Core::RootSignature::ShaderType::VERTEX:
				errorString += L"顶点着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::HULL:
				errorString += L"外壳着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::DOMAIN:
				errorString += L"域着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::GEOMETRY:
				errorString += L"几何着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::PIXEL:
				errorString += L"像素着色器";
				break;
			case D3D12Core::RootSignature::ShaderType::COMPUTE:
				errorString += L"计算着色器";
				break;
			default:
				break;
			}

			errorString += L"分配了" + std::to_wstring(numShaderConstants) + L"个可写入常数，但侦测到了" + std::to_wstring(numWrite) + L"个常量写入。请检查你的代码实现！";

			LOGERROR(errorString);
		}
	}

	void GraphicsContext::setVSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::VERTEX, numValues);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(graphicsRootSignature->getVSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setHSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::HULL, numValues);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(graphicsRootSignature->getHSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setDSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::DOMAIN, numValues);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(graphicsRootSignature->getDSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setGSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::GEOMETRY, numValues);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(graphicsRootSignature->getGSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setPSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::PIXEL, numValues);
#endif // _DEBUG

		commandList->setGraphicsRootConstants(graphicsRootSignature->getPSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setCSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const
	{
#ifdef _DEBUG
		constantsWriteCheck(D3D12Core::RootSignature::ShaderType::COMPUTE, numValues);
#endif // _DEBUG

		commandList->setComputeRootConstants(computeRootSignature->getCSConstantsParameterIndex(), numValues, data, offset);

		offset += numValues;
	}

	void GraphicsContext::setVSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ graphicsRootSignature->getVSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setHSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ graphicsRootSignature->getHSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setDSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ graphicsRootSignature->getDSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setGSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ graphicsRootSignature->getGSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setPSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ graphicsRootSignature->getPSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setCSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		Resource::D3D12Resource::Buffer* const buffer = immutableCBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		pushRootConstantBufferDesc({ computeRootSignature->getCSConstantBufferParameterIndex(),immutableCBuffer.getGPUAddress() });
	}

	void GraphicsContext::setPipelineState(const D3D12Core::PipelineState& pipelineState)
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

	void GraphicsContext::setRenderTargets(const Resource::D3D12Resource::DepthStencilDesc& depthStencil)
	{
#ifdef _DEBUG
		if (nullptr == depthStencil.texture)
		{
			LOGERROR(L"if you want to bind only depth stencil view then texture pointer cannot be nullptr");
		}
#endif // _DEBUG

		setResourceState(depthStencil);

		depthStencilClearDesc.setHandle(depthStencil.dsvHandle);

		commandList->setRenderTargets(0, nullptr, FALSE, &depthStencil.dsvHandle);
	}

	void GraphicsContext::setDefRenderTarget() const
	{
		commandList->setDefRenderTarget();
	}

	void GraphicsContext::clearDefRenderTarget(const float clearValue[4]) const
	{
		commandList->clearRenderTarget(Graphics::getBackBufferHandle(), clearValue, 0, nullptr);
	}

	void GraphicsContext::clearRenderTarget(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4])
	{
		renderTargetClearDescs.emplace_back(RenderTargetClearDesc{ desc.rtvHandle,{clearValue[0],clearValue[1],clearValue[2],clearValue[3]} });
	}

	void GraphicsContext::clearRenderTargetInstant(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4])
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

	void GraphicsContext::clearDepthStencilInstant(const Resource::D3D12Resource::DepthStencilDesc& desc, const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil)
	{
		setResourceState(desc);

		transitionResources();

		depthStencilClearDesc.setHandle(desc.dsvHandle);

		depthStencilClearDesc.setClearData(flags, depth, stencil);

		resetDepthStencilClearDescs();
	}

	void GraphicsContext::setIndexBuffer(const Resource::D3D12Resource::IndexBufferDesc& indexBuffer)
	{
		setResourceState(indexBuffer);

		commandList->setIndexBuffer(&indexBuffer.ibv);
	}

	void GraphicsContext::setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology)
	{
		if (topology != primitiveTopology)
		{
			primitiveTopology = topology;

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

	void GraphicsContext::setScissorRect(const uint32_t left, const uint32_t top, const uint32_t right, const uint32_t bottom)
	{
		rt.left = left;
		rt.top = top;
		rt.right = right;
		rt.bottom = bottom;

		commandList->setScissorRects(1, &rt);
	}

	void GraphicsContext::setScissorRect(const float left, const float top, const float right, const float bottom)
	{
		setScissorRect(static_cast<uint32_t>(left), static_cast<uint32_t>(top), static_cast<uint32_t>(right), static_cast<uint32_t>(bottom));
	}

	void GraphicsContext::setViewportSimple(const float width, const float height)
	{
		setViewport(width, height);
		setScissorRect(0, 0, width, height);
	}

	void GraphicsContext::setViewportSimple(const uint32_t width, const uint32_t height)
	{
		setViewport(width, height);
		setScissorRect(0, 0, width, height);
	}

	void GraphicsContext::clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const float values[4])
	{
		Resource::D3D12Resource::D3D12ResourceBase* resource = setResourceState(desc);

		transitionResources();

		uavBarrier({ resource });

		commandList->clearUnorderedAccessFloat(desc.viewGPUHandle, desc.viewCPUHandle, resource->getResource(), values, 0, nullptr);

		uavBarrier({ resource });
	}

	void GraphicsContext::clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const uint32_t values[4])
	{
		Resource::D3D12Resource::D3D12ResourceBase* resource = setResourceState(desc);

		transitionResources();

		uavBarrier({ resource });

		commandList->clearUnorderedAccessUint(desc.viewGPUHandle, desc.viewCPUHandle, resource->getResource(), values, 0, nullptr);

		uavBarrier({ resource });
	}

	void GraphicsContext::draw(const uint32_t vertexCountPerInstance, const uint32_t instanceCount, const uint32_t startVertexLocation, const uint32_t startInstanceLocation)
	{
		transitionResources();

		flushRootConstantBufferDescs(true);

		flushRenderTargetClearDescs();

		resetDepthStencilClearDescs();

		commandList->drawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	}

	void GraphicsContext::drawIndexed(const uint32_t indexCountPerInstance, const uint32_t instanceCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation, const uint32_t startInstanceLocation)
	{
		transitionResources();

		flushRootConstantBufferDescs(true);

		flushRenderTargetClearDescs();

		resetDepthStencilClearDescs();

		commandList->drawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	void GraphicsContext::dispatch(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ)
	{
		transitionResources();

		flushRootConstantBufferDescs(false);

		commandList->dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
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

		resetUserDefinedGlobalConstantBuffer();
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

	D3D12Core::CommandList* GraphicsContext::getCommandList() const
	{
		return commandList.get();
	}

	void GraphicsContext::transitionResources()
	{
		commandList->transitionResources();
	}

	void GraphicsContext::pushRootConstantBufferDesc(const RootConstantBufferDesc& desc)
	{
		rootConstantBufferDescs.emplace_back(desc);
	}

	void GraphicsContext::flushRootConstantBufferDescs(const bool isGraphicsRootSignature)
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

	void GraphicsContext::flushRenderTargetClearDescs()
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

	void GraphicsContext::resetDepthStencilClearDescs()
	{
		if (depthStencilClearDesc.needClear)
		{
			commandList->clearDepthStencil(depthStencilClearDesc.handle, depthStencilClearDesc.flags, depthStencilClearDesc.depth, depthStencilClearDesc.stencil, 0, nullptr);

			depthStencilClearDesc.reset();
		}
	}

	void GraphicsContext::setGraphicsRootSignature(const D3D12Core::RootSignature* const rootSignature)
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

	void GraphicsContext::setComputeRootSignature(const D3D12Core::RootSignature* const rootSignature)
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

	void GraphicsContext::setResourceState(const Resource::D3D12Resource::ShaderResourceDesc& desc, const uint32_t targetSRVState)
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

	void GraphicsContext::setResourceState(const Resource::D3D12Resource::RenderTargetDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.texture, desc.mipSlice, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	void GraphicsContext::setResourceState(const Resource::D3D12Resource::DepthStencilDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.texture, desc.mipSlice, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	void GraphicsContext::setResourceState(const Resource::D3D12Resource::VertexBufferDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	void GraphicsContext::setResourceState(const Resource::D3D12Resource::IndexBufferDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.buffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	Resource::D3D12Resource::D3D12ResourceBase* GraphicsContext::setResourceState(const Resource::D3D12Resource::ClearUAVDesc& desc)
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

	void GraphicsContext::resetGraphicsRootSignature()
	{
		graphicsRootSignature = nullptr;
	}

	void GraphicsContext::resetPrimitiveTopology()
	{
		primitiveTopology = TOPOLOGY::UNDEFINED;
	}

	void GraphicsContext::resetComputeRootSignature()
	{
		computeRootSignature = nullptr;
	}

	void GraphicsContext::resetUserDefinedGlobalConstantBuffer()
	{
		userDefinedGlobalConstantBuffer = nullptr;
	}

	void GraphicsContext::DepthStencilClearDesc::setHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
	{
		this->handle = handle;
	}

	void GraphicsContext::DepthStencilClearDesc::setClearData(const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil)
	{
		this->flags = flags;

		this->depth = depth;

		this->stencil = stencil;

		this->needClear = true;
	}

	void GraphicsContext::DepthStencilClearDesc::reset()
	{
		this->needClear = false;
	}
}
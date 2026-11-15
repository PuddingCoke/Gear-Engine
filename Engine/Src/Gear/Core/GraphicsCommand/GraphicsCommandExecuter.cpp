#include <Gear/Core/GraphicsCommand/GraphicsCommandExecuter.h>

#include <Gear/Core/RenderEngine.h>

#include <Gear/Core/Graphics.h>

#include <Gear/Core/TOPOLOGY.h>

#include <Gear/Utils/Math.h>

namespace Gear::Core
{
	static constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE getPrimitiveTopologyType(
		const D3D_PRIMITIVE_TOPOLOGY topology)
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

	static constexpr std::array<D3D12_PRIMITIVE_TOPOLOGY_TYPE, primitiveTopologyTypeTableLength>
		primitiveTopologyTypeTable = []() constexpr
	{
		std::array<D3D12_PRIMITIVE_TOPOLOGY_TYPE, primitiveTopologyTypeTableLength> table;

		for (uint32_t i = 0; i < primitiveTopologyTypeTableLength; i++)
		{
			table[i] = getPrimitiveTopologyType(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(i));
		}

		return table;
	}();

	GraphicsCommandExecuter::GraphicsCommandExecuter() :
		vp{ 0.f, 0.f, 0.f, 0.f, 0.f, 1.f },
		rt{ 0, 0, 0, 0 },
		commandList(makeUnique<D3D12Core::GraphicsCommandList>())
	{
		resetTrackedStates();
	}

	void GraphicsCommandExecuter::updateBuffer(D3D12Resource::Buffer* const buffer,
	                                           D3D12Resource::UploadHeap* const uploadHeap,
	                                           const uint64_t size) const
	{
		commandList->copyBufferRegion(buffer, 0, uploadHeap, 0, size);
	}

	void GraphicsCommandExecuter::setPipelineState(D3D12Core::GraphicsState& graphicsPipelineState)
	{
		graphicsState = &graphicsPipelineState;

		setGraphicsRootSignature(graphicsState->getRootSignature());
	}

	void GraphicsCommandExecuter::setPipelineState(D3D12Core::ComputeState& computePipelineState)
	{
		computeState = &computePipelineState;

		setPipelineState(computeState->getPipelineState());

		setComputeRootSignature(computeState->getRootSignature());
	}

	void GraphicsCommandExecuter::setRenderTargets(const uint64_t renderTargetDescsPtrOffset,
	                                               const uint32_t numRenderTargets,
	                                               const Resource::DepthStencilDesc& depthStencil)
	{
#ifdef _DEBUG
		if (numRenderTargets > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
		{
			THROWLOG(LOGERROR() << "无法绑定 8 个以上的渲染目标");
		}
#endif // _DEBUG

		switchGraphicsPipelineState = true;

		transientNumRTV = numRenderTargets;

		for (uint32_t i = 0; i < numRenderTargets; i++)
		{
			transientRTVHandles[i] = renderTargetDescs[renderTargetDescsPtrOffset + i].rtvHandle;

			transientRTVFormats[i] = renderTargetDescs[renderTargetDescsPtrOffset + i].rtvFormat;

			setResourceState(renderTargetDescs[renderTargetDescsPtrOffset + i]);
		}

		if (numRenderTargets)
		{
			if (depthStencil.texture)
			{
				setResourceState(depthStencil);

				depthStencilClearDesc.setHandle(depthStencil.dsvHandle);

				transientDSVFormat = depthStencil.dsvFormat;

				commandList->setRenderTargets(static_cast<uint32_t>(transientNumRTV),
				                              transientRTVHandles.data(),
				                              FALSE,
				                              &(depthStencil.dsvHandle));
			}
			else
			{
				transientDSVFormat = FMT::UNKNOWN;

				commandList->setRenderTargets(
					static_cast<uint32_t>(transientNumRTV), transientRTVHandles.data(), FALSE, nullptr);
			}
		}
		else
		{
#ifdef _DEBUG
			if (depthStencil.texture == nullptr)
			{
				THROWLOG(LOGERROR() << "无法绑定空的深度模板缓冲");
			}
#endif
			setResourceState(depthStencil);

			depthStencilClearDesc.setHandle(depthStencil.dsvHandle);

			switchGraphicsPipelineState = true;

			transientNumRTV = 0;

			transientDSVFormat = depthStencil.dsvFormat;

			commandList->setRenderTargets(0, nullptr, FALSE, &depthStencil.dsvHandle);
		}
	}

	void GraphicsCommandExecuter::setDefRenderTarget()
	{
		commandList->setDefRenderTarget();

		switchGraphicsPipelineState = true;

		transientNumRTV = 1;

		transientRTVFormats[0] = Graphics::backBufferFormat;

		transientDSVFormat = FMT::UNKNOWN;
	}

	void GraphicsCommandExecuter::clearDefRenderTarget(const float clearValue[4]) const
	{
		commandList->clearDefRenderTarget(clearValue);
	}

	void GraphicsCommandExecuter::clearRenderTarget(const Resource::RenderTargetDesc& desc,
	                                                const float clearValue[4])
	{
		renderTargetClearDescs.push(RenderTargetClearDesc{
			desc.rtvHandle, { clearValue[0], clearValue[1], clearValue[2], clearValue[3] } });
	}

	void GraphicsCommandExecuter::clearRenderTargetInstant(const Resource::RenderTargetDesc& desc,
	                                                       const float clearValue[4])
	{
		clearRenderTarget(desc, clearValue);

		setResourceState(desc);

		commandList->flushResourceBarriers();

		flushRenderTargetClearDescs();
	}

	void GraphicsCommandExecuter::clearDepthStencil(const D3D12_CLEAR_FLAGS flags,
	                                                const float depth,
	                                                const uint8_t stencil)
	{
		depthStencilClearDesc.setClearData(flags, depth, stencil);
	}

	void GraphicsCommandExecuter::clearDepthStencilInstant(const Resource::DepthStencilDesc& desc,
	                                                       const D3D12_CLEAR_FLAGS flags,
	                                                       const float depth,
	                                                       const uint8_t stencil)
	{
		setResourceState(desc);

		commandList->flushResourceBarriers();

		depthStencilClearDesc.setHandle(desc.dsvHandle);

		depthStencilClearDesc.setClearData(flags, depth, stencil);

		resetDepthStencilClearDesc();
	}

	void GraphicsCommandExecuter::setVertexBuffers(const uint64_t vertexBufferDescsPtrOffset,
	                                               const uint32_t numVertexBuffers,
	                                               const uint32_t startSlot)
	{
#ifdef _DEBUG
		if (startSlot + numVertexBuffers > D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT)
		{
			THROWLOG(LOGERROR() << "无法绑定更多的顶点缓冲");
		}
#endif // _DEBUG

		for (uint32_t i = 0; i < numVertexBuffers; i++)
		{
			transientVBViews[i] = vertexBufferDescs[vertexBufferDescsPtrOffset + i].vbv;

			setResourceState(vertexBufferDescs[vertexBufferDescsPtrOffset + i]);
		}

		commandList->setVertexBuffers(startSlot, numVertexBuffers, transientVBViews.data());
	}

	void GraphicsCommandExecuter::setIndexBuffer(const Resource::IndexBufferDesc& indexBuffer)
	{
		setResourceState(indexBuffer);

		commandList->setIndexBuffer(&indexBuffer.ibv);
	}

	void GraphicsCommandExecuter::setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology)
	{
		if (primitiveTopology != topology)
		{
			primitiveTopology = topology;

			transientPrimitiveTopologyType = primitiveTopologyTypeTable[topology];

			commandList->setPrimitiveTopology(primitiveTopology);
		}
	}

	void GraphicsCommandExecuter::setViewport(const float width, const float height)
	{
		vp.Width = width;

		vp.Height = height;

		commandList->setViewports(1, &vp);
	}

	void GraphicsCommandExecuter::setScissorRect(const int32_t left,
	                                             const int32_t top,
	                                             const int32_t right,
	                                             const int32_t bottom)
	{
		rt.left = left;

		rt.top = top;

		rt.right = right;

		rt.bottom = bottom;

		commandList->setScissorRects(1, &rt);
	}

	void GraphicsCommandExecuter::setPixelShaderResources(const uint64_t pixelShaderResourceDescPtrOffset,
	                                                      const uint32_t numPixelShaderResourceDesc)
	{
		setShaderResources(pixelShaderResourceDescs.data() + pixelShaderResourceDescPtrOffset,
		                   numPixelShaderResourceDesc,
		                   D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void GraphicsCommandExecuter::setNonPixelShaderResources(
		const uint64_t nonPixelShaderResourceDescPtrOffset,
		const uint32_t numNonPixelShaderResourceDesc)
	{
		setShaderResources(nonPixelShaderResourceDescs.data() + nonPixelShaderResourceDescPtrOffset,
		                   numNonPixelShaderResourceDesc,
		                   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	void GraphicsCommandExecuter::draw(const uint32_t vertexCountPerInstance,
	                                   const uint32_t instanceCount,
	                                   const uint32_t startVertexLocation,
	                                   const uint32_t startInstanceLocation)
	{
		prepareDrawCallResources();

		commandList->drawInstanced(
			vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	}

	void GraphicsCommandExecuter::drawIndexed(const uint32_t indexCountPerInstance,
	                                          const uint32_t instanceCount,
	                                          const uint32_t startIndexLocation,
	                                          const int32_t baseVertexLocation,
	                                          const uint32_t startInstanceLocation)
	{
		prepareDrawCallResources();

		commandList->drawIndexedInstanced(indexCountPerInstance,
		                                  instanceCount,
		                                  startIndexLocation,
		                                  baseVertexLocation,
		                                  startInstanceLocation);
	}

	void GraphicsCommandExecuter::dispatchGrp(const uint32_t threadGroupCountX,
	                                          const uint32_t threadGroupCountY,
	                                          const uint32_t threadGroupCountZ)
	{
		prepareDispatchCallResources();

		commandList->dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	void GraphicsCommandExecuter::dispatchDim(const uint32_t dispatchThreadCountX,
	                                          const uint32_t dispatchThreadCountY,
	                                          const uint32_t dispatchThreadCountZ)
	{
		const DirectX::XMUINT3 groupDimension =
			computeState->getPipelineStateData().computeData.groupDimension;

		dispatchGrp(Utils::Math::divCeil(dispatchThreadCountX, groupDimension.x),
		            Utils::Math::divCeil(dispatchThreadCountY, groupDimension.y),
		            Utils::Math::divCeil(dispatchThreadCountZ, groupDimension.z));
	}

	void GraphicsCommandExecuter::clearUnorderedAccess(const Resource::UAVClearDesc& desc,
	                                                   const float values[4])
	{
		D3D12Resource::D3D12ResourceBase* resource = setResourceState(desc);

		commandList->flushTransitionResources();

		commandList->pushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource()));

		commandList->flushResourceBarriers();

		commandList->clearUnorderedAccessFloat(
			*desc.viewGPUHandle, desc.viewCPUHandle, resource->getResource(), values, 0, nullptr);

		commandList->pushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource()));

		commandList->flushResourceBarriers();
	}

	void GraphicsCommandExecuter::clearUnorderedAccess(const Resource::UAVClearDesc& desc,
	                                                   const uint32_t values[4])
	{
		D3D12Resource::D3D12ResourceBase* resource = setResourceState(desc);

		commandList->flushTransitionResources();

		commandList->pushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource()));

		commandList->flushResourceBarriers();

		commandList->clearUnorderedAccessUint(
			*desc.viewGPUHandle, desc.viewCPUHandle, resource->getResource(), values, 0, nullptr);

		commandList->pushResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource()));

		commandList->flushResourceBarriers();
	}

	void GraphicsCommandExecuter::begin()
	{
		commandList->open();

		commandList->setDescriptorHeap(GlobalDescriptorHeap::getResourceHeap()->get(),
		                               GlobalDescriptorHeap::getSamplerHeap()->get());

		switchGraphicsPipelineState = false;

		resetTrackedStates();

		for (uint32_t i = 0; i < static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT); i++)
		{
			shaderConstants[i].clear();
		}

		pixelShaderResourceDescs.clear();

		nonPixelShaderResourceDescs.clear();

		renderTargetDescs.clear();

		vertexBufferDescs.clear();
	}

	void GraphicsCommandExecuter::resetTrackedStates()
	{
		resetPipelineState();

		resetTrackedGraphicsStates();

		resetTrackedComputeStates();
	}

	void GraphicsCommandExecuter::resetPipelineState()
	{
		currentPipelineState = nullptr;
	}

	void GraphicsCommandExecuter::resetTrackedGraphicsStates()
	{
		resetGraphicsRootSignature();

		resetPrimitiveTopology();
	}

	void GraphicsCommandExecuter::resetTrackedComputeStates()
	{
		resetComputeRootSignature();
	}

	D3D12Core::GraphicsCommandList* GraphicsCommandExecuter::getCommandList() const
	{
		return commandList.get();
	}

	void GraphicsCommandExecuter::addNonPersistentResource(Resource::ResourceBase* const resource)
	{
#ifdef _DEBUG
		if (!resource)
		{
			THROWLOG(LOGERROR() << "资源不能为空");
		}

		if (resource->getPersistent())
		{
			THROWLOG(LOGERROR() << "对于持久性资源来说，调用addNonPersistentResource是非法的");
		}
#endif

		if (!resource->getInCopyingList())
		{
			nonPersistentResources.push_back(resource);

			resource->pushToCopyingList();
		}
	}

	void GraphicsCommandExecuter::addUnsettedIndex(const D3D12Core::ShaderType shaderType,
	                                               uint32_t* indexAddress,
	                                               const uint64_t location)
	{
		unsettedIndices[static_cast<uint32_t>(shaderType)].push_back({ indexAddress, location });
	}

	uint64_t GraphicsCommandExecuter::getShaderConstantsWriteLocation(const D3D12Core::ShaderType shaderType)
	{
		return shaderConstants[static_cast<uint32_t>(shaderType)].size();
	}

	uint64_t GraphicsCommandExecuter::getPixelShaderResourceDescsWriteLocation()
	{
		return pixelShaderResourceDescs.size();
	}

	uint64_t GraphicsCommandExecuter::getNonPixelShaderResourceDescsWriteLocation()
	{
		return nonPixelShaderResourceDescs.size();
	}

	uint64_t GraphicsCommandExecuter::getRenderTargetDescsWriteLocation()
	{
		return renderTargetDescs.size();
	}

	uint64_t GraphicsCommandExecuter::getVertexBufferDescsWriteLocation()
	{
		return vertexBufferDescs.size();
	}

	void GraphicsCommandExecuter::pushShaderConstants(const D3D12Core::ShaderType shaderType,
	                                                  const uint32_t* const srcBegin,
	                                                  const uint32_t count)
	{
		shaderConstants[static_cast<uint32_t>(shaderType)].insert(
			shaderConstants[static_cast<uint32_t>(shaderType)].end(), srcBegin, srcBegin + count);
	}

	void GraphicsCommandExecuter::pushPixelShaderResourceDesc(const Resource::ShaderResourceDesc& desc)
	{
		pixelShaderResourceDescs.push_back(desc);
	}

	void GraphicsCommandExecuter::pushNonPixelShaderResourceDesc(const Resource::ShaderResourceDesc& desc)
	{
		nonPixelShaderResourceDescs.push_back(desc);
	}

	void GraphicsCommandExecuter::pushRenderTargetDesc(const Resource::RenderTargetDesc& desc)
	{
		renderTargetDescs.push_back(desc);
	}

	void GraphicsCommandExecuter::pushVertexBufferDesc(const Resource::VertexBufferDesc& desc)
	{
		vertexBufferDescs.push_back(desc);
	}

	void GraphicsCommandExecuter::copyDescriptors()
	{
		if (nonPersistentResources.size())
		{
			for (Resource::ResourceBase* const resource : nonPersistentResources)
			{
				resource->copyDescriptors();

				resource->popFromCopyingList();
			}

			nonPersistentResources.clear();
		}
	}

	void GraphicsCommandExecuter::resolveDescriptorIndices()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT); i++)
		{
			for (const auto& index : unsettedIndices[i])
			{
				if (index.location < shaderConstants[i].size())
				{
					shaderConstants[i][index.location] = *index.indexAddress;
				}
			}

			unsettedIndices[i].clear();
		}
	}

	void GraphicsCommandExecuter::setPipelineState(ID3D12PipelineState* const pipelineState)
	{
		if (currentPipelineState != pipelineState)
		{
			currentPipelineState = pipelineState;

			commandList->setPipelineState(currentPipelineState);
		}
	}

	void GraphicsCommandExecuter::setShaderResources(const Resource::ShaderResourceDesc* const descs,
	                                                 const uint32_t numDescs,
	                                                 const uint32_t targetSRVState)
	{
		for (uint32_t i = 0; i < numDescs; i++)
		{
			const Resource::ShaderResourceDesc& desc = descs[i];

			setResourceState(desc, targetSRVState);
		}
	}

	void GraphicsCommandExecuter::pushRootConstantBufferDesc(const RootConstantBufferDesc& desc)
	{
		rootConstantBufferDescs.push(desc);
	}

	void GraphicsCommandExecuter::flushRenderTargetClearDescs()
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

	void GraphicsCommandExecuter::resetDepthStencilClearDesc()
	{
		if (depthStencilClearDesc.getFlags())
		{
			commandList->clearDepthStencil(depthStencilClearDesc.getHandle(),
			                               depthStencilClearDesc.getFlags(),
			                               depthStencilClearDesc.getDepth(),
			                               depthStencilClearDesc.getStencil(),
			                               0,
			                               nullptr);

			depthStencilClearDesc.reset();
		}
	}

	void GraphicsCommandExecuter::setGraphicsRootSignature(
		const D3D12Core::RootSignature* const rootSignature)
	{
		if (graphicsRootSignature != rootSignature)
		{
			graphicsRootSignature = rootSignature;

			commandList->setGraphicsRootSignature(rootSignature->get());

			commandList->setGraphicsRootConstantBuffer(
				D3D12Core::RootSignature::getEngineGlobalConstantBufferParameterIndex(),
				RenderEngine::getEngineGlobalCBuffer()->getGPUAddress());
		}
	}

	void GraphicsCommandExecuter::setComputeRootSignature(const D3D12Core::RootSignature* const rootSignature)
	{
		if (computeRootSignature != rootSignature)
		{
			computeRootSignature = rootSignature;

			commandList->setComputeRootSignature(rootSignature->get());

			commandList->setComputeRootConstantBuffer(
				D3D12Core::RootSignature::getEngineGlobalConstantBufferParameterIndex(),
				RenderEngine::getEngineGlobalCBuffer()->getGPUAddress());
		}
	}

	const D3D12Core::RootSignature* GraphicsCommandExecuter::getGraphicsRootSignature() const
	{
#ifdef _DEBUG
		if (graphicsRootSignature == nullptr)
		{
			THROWLOG(LOGERROR() << "没有检测到图形根签名绑定，是否忘记设置管线状态？");
		}
#endif // _DEBUG

		return graphicsRootSignature;
	}

	const D3D12Core::RootSignature* GraphicsCommandExecuter::getComputeRootSignature() const
	{
#ifdef _DEBUG
		if (computeRootSignature == nullptr)
		{
			THROWLOG(LOGERROR() << "没有检测到计算根签名绑定，是否忘记设置管线状态？");
		}
#endif // _DEBUG

		return computeRootSignature;
	}

	void GraphicsCommandExecuter::setResourceState(const Resource::ShaderResourceDesc& desc,
	                                               const uint32_t targetSRVState)
	{
		if (desc.type == Resource::ShaderResourceDesc::BUFFER)
		{
			if (desc.state == Resource::ShaderResourceDesc::SRV)
			{
				commandList->trackAndSetResourceState(desc.bufferDesc.buffer, targetSRVState);
			}
			else if (desc.state == Resource::ShaderResourceDesc::UAV)
			{
				commandList->trackAndSetResourceState(desc.bufferDesc.buffer,
				                                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				commandList->trackAndSetResourceState(desc.bufferDesc.counterBuffer,
				                                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			}
			else
			{
				commandList->trackAndSetResourceState(desc.bufferDesc.buffer,
				                                      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			}
		}
		else if (desc.type == Resource::ShaderResourceDesc::TEXTURE)
		{
			if (desc.state == Resource::ShaderResourceDesc::SRV)
			{
				commandList->trackAndSetResourceState(
					desc.textureDesc.texture, desc.textureDesc.mipSlice, targetSRVState);
			}
			else if (desc.state == Resource::ShaderResourceDesc::UAV)
			{
				commandList->trackAndSetResourceState(desc.textureDesc.texture,
				                                      desc.textureDesc.mipSlice,
				                                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			}
		}
	}

	void GraphicsCommandExecuter::setResourceState(const Resource::RenderTargetDesc& desc)
	{
		commandList->trackAndSetResourceState(
			desc.texture, desc.mipSlice, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	void GraphicsCommandExecuter::setResourceState(const Resource::DepthStencilDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.texture, desc.mipSlice, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	void GraphicsCommandExecuter::setResourceState(const Resource::VertexBufferDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	void GraphicsCommandExecuter::setResourceState(const Resource::IndexBufferDesc& desc)
	{
		commandList->trackAndSetResourceState(desc.buffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	D3D12Resource::D3D12ResourceBase* GraphicsCommandExecuter::setResourceState(
		const Resource::UAVClearDesc& desc)
	{
		D3D12Resource::D3D12ResourceBase* resource = nullptr;

		if (desc.type == Resource::UAVClearDesc::BUFFER)
		{
			resource = desc.bufferDesc.buffer;

			commandList->trackAndSetResourceState(desc.bufferDesc.buffer,
			                                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
		else if (desc.type == Resource::UAVClearDesc::TEXTURE)
		{
			resource = desc.textureDesc.texture;

			commandList->trackAndSetResourceState(
				desc.textureDesc.texture, desc.textureDesc.mipSlice, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		return resource;
	}

	void GraphicsCommandExecuter::prepareDrawCallResources()
	{
		if (switchGraphicsPipelineState)
		{
			graphicsState->updatePipelineState(transientRTVFormats.data(),
			                                   transientNumRTV,
			                                   transientDSVFormat,
			                                   transientPrimitiveTopologyType);

			setPipelineState(graphicsState->getPipelineState());

			switchGraphicsPipelineState = false;
		}

		commandList->flushResourceBarriers();

		flushRootConstantBufferDescs<true>();

		flushRenderTargetClearDescs();

		resetDepthStencilClearDesc();
	}

	void GraphicsCommandExecuter::prepareDispatchCallResources()
	{
		commandList->flushResourceBarriers();

		flushRootConstantBufferDescs<false>();
	}

	void GraphicsCommandExecuter::resetGraphicsRootSignature()
	{
		graphicsRootSignature = nullptr;
	}

	void GraphicsCommandExecuter::resetPrimitiveTopology()
	{
		primitiveTopology = TOPOLOGY::UNDEFINED;

		transientPrimitiveTopologyType = TOPOLOGY::TYPE::UNDEFINED;
	}

	void GraphicsCommandExecuter::resetComputeRootSignature()
	{
		computeRootSignature = nullptr;
	}

	void GraphicsCommandExecuter::constantsWriteCheck(const D3D12Core::ShaderType shaderType,
	                                                  const uint32_t numWrite,
	                                                  const uint32_t offset) const
	{
		uint32_t numShaderConstants = 0u;

		switch (shaderType)
		{
		case D3D12Core::ShaderType::VERTEX:
		case D3D12Core::ShaderType::HULL:
		case D3D12Core::ShaderType::DOMAIN:
		case D3D12Core::ShaderType::GEOMETRY:
		case D3D12Core::ShaderType::PIXEL:
		case D3D12Core::ShaderType::AMPLIFICATION:
		case D3D12Core::ShaderType::MESH:
			numShaderConstants = getGraphicsRootSignature()->getNumShaderConstants(shaderType);
			break;
		case D3D12Core::ShaderType::COMPUTE:
			numShaderConstants = getComputeRootSignature()->getNumShaderConstants(shaderType);
			break;
		default:
			break;
		}

		if (numShaderConstants < numWrite + offset)
		{
			std::string shaderName = "";

			switch (shaderType)
			{
			case D3D12Core::ShaderType::VERTEX:
				shaderName = "顶点着色器";
				break;
			case D3D12Core::ShaderType::HULL:
				shaderName = "外壳着色器";
				break;
			case D3D12Core::ShaderType::DOMAIN:
				shaderName = "域着色器";
				break;
			case D3D12Core::ShaderType::GEOMETRY:
				shaderName = "几何着色器";
				break;
			case D3D12Core::ShaderType::PIXEL:
				shaderName = "像素着色器";
				break;
			case D3D12Core::ShaderType::COMPUTE:
				shaderName = "计算着色器";
				break;
			case D3D12Core::ShaderType::AMPLIFICATION:
				shaderName = "放大着色器";
				break;
			case D3D12Core::ShaderType::MESH:
				shaderName = "网格着色器";
				break;
			default:
				break;
			}

			THROWLOG(LOGERROR() << "侦测到常量写入终止位置" << offset + numWrite - 1u << "越界" << shaderName
			                    << "的常量写入终止位置最高为" << numShaderConstants - 1u);
		}
	}

	void GraphicsCommandExecuter::DepthStencilClearDesc::setHandle(
		const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) noexcept
	{
		handle = cpuHandle;
	}

	void GraphicsCommandExecuter::DepthStencilClearDesc::setClearData(const D3D12_CLEAR_FLAGS clearFlags,
	                                                                  const float clearDepth,
	                                                                  const uint8_t clearStencil) noexcept
	{
		flags = clearFlags;

		depth = clearDepth;

		stencil = clearStencil;
	}

	void GraphicsCommandExecuter::DepthStencilClearDesc::reset() noexcept
	{
		flags = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GraphicsCommandExecuter::DepthStencilClearDesc::getHandle() const noexcept
	{
		return handle;
	}

	D3D12_CLEAR_FLAGS GraphicsCommandExecuter::DepthStencilClearDesc::getFlags() const noexcept
	{
		return static_cast<D3D12_CLEAR_FLAGS>(flags);
	}

	float GraphicsCommandExecuter::DepthStencilClearDesc::getDepth() const noexcept
	{
		return depth;
	}

	uint8_t GraphicsCommandExecuter::DepthStencilClearDesc::getStencil() const noexcept
	{
		return stencil;
	}

}

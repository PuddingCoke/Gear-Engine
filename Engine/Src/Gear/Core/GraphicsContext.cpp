#include <Gear/Core/GraphicsContext.h>

#include <Gear/Core/RenderEngine.h>

#include <Gear/Core/Graphics.h>

#include <Gear/Core/TOPOLOGY.h>

#include <Gear/Utils/Math.h>

#include <Gear/Core/GraphicsCommand/ClearDepthStencilCommandBase.h>

#include <Gear/Core/GraphicsCommand/ClearRenderTargetCommandBase.h>

#include <Gear/Core/GraphicsCommand/ClearUnorderedAccessCommandBase.h>

#include <Gear/Core/GraphicsCommand/DispatchCommandBase.h>

#include <Gear/Core/GraphicsCommand/DrawCommandBase.h>

#include <Gear/Core/GraphicsCommand/SetIndexBufferCommand.h>

#include <Gear/Core/GraphicsCommand/SetPipelineStateCommandBase.h>

#include <Gear/Core/GraphicsCommand/SetPrimitiveTopologyCommand.h>

#include <Gear/Core/GraphicsCommand/SetScissorRectCommand.h>

#include <Gear/Core/GraphicsCommand/SetViewportCommand.h>

#include <Gear/Core/GraphicsCommand/UpdateBufferCommand.h>

namespace Gear::Core
{

	GraphicsContext::GraphicsContext() : graphicsCommandExecuter(makeUnique<GraphicsCommandExecuter>())
	{
	}

	GraphicsContext::~GraphicsContext()
	{
	}

	void GraphicsContext::updateBuffer(Resource::BufferView& bufferView,
	                                   const void* const data,
	                                   const uint32_t size)
	{
		const Resource::BufferView::UpdateStruct updateStruct = bufferView.getUpdateStruct(data, size);

		auto* const command = UpdateBufferCommand::acquire();

		command->set(updateStruct.buffer, updateStruct.uploadHeap, size);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::updateBuffer(Resource::DefaultCBuffer& defaultCBuffer,
	                                   const void* const data,
	                                   const uint32_t size)
	{
		const Resource::DefaultCBuffer::UpdateStruct updateStruct =
			defaultCBuffer.getUpdateStruct(data, size);

		auto* const command = UpdateBufferCommand::acquire();

		command->set(updateStruct.buffer, updateStruct.uploadHeap, size);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setVSConstants(const uint32_t numValues, const void* const data, uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::VERTEX>(numValues, data, offset);
	}

	void GraphicsContext::setHSConstants(const uint32_t numValues, const void* const data, uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::HULL>(numValues, data, offset);
	}

	void GraphicsContext::setDSConstants(const uint32_t numValues, const void* const data, uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::DOMAIN>(numValues, data, offset);
	}

	void GraphicsContext::setGSConstants(const uint32_t numValues, const void* const data, uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::GEOMETRY>(numValues, data, offset);
	}

	void GraphicsContext::setPSConstants(const uint32_t numValues, const void* const data, uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::PIXEL>(numValues, data, offset);
	}

	void GraphicsContext::setCSConstants(const uint32_t numValues, const void* const data, uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::COMPUTE>(numValues, data, offset);
	}

	void GraphicsContext::setGlobalConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		auto* const command = SetAllShaderConstantBufferCommand::acquire();

		command->set(immutableCBuffer);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setVSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		auto* const command = SetVSConstantBufferCommand::acquire();

		command->set(immutableCBuffer);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setHSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		auto* const command = SetHSConstantBufferCommand::acquire();

		command->set(immutableCBuffer);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setDSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		auto* const command = SetDSConstantBufferCommand::acquire();

		command->set(immutableCBuffer);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setGSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		auto* const command = SetGSConstantBufferCommand::acquire();

		command->set(immutableCBuffer);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setPSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		auto* const command = SetPSConstantBufferCommand::acquire();

		command->set(immutableCBuffer);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setCSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer)
	{
		auto* const command = SetCSConstantBufferCommand::acquire();

		command->set(immutableCBuffer);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setPipelineState(D3D12Core::GraphicsState& graphicsPipelineState)
	{
		auto* const command = SetGraphicsPipelineStateCommand::acquire();

		command->set(graphicsPipelineState);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setPipelineState(D3D12Core::ComputeState& computePipelineState)
	{
		auto* const command = SetComputePipelineStateCommand::acquire();

		command->set(computePipelineState);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setRenderTargets(const Resource::DepthStencilDesc& depthStencil)
	{
		auto* const command = SetRenderTargetsCommand::acquire();

		command->set(renderTargetDescsPtrOffset, 0, depthStencil);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setDefRenderTarget()
	{
		auto* const command = SetDefRenderTargetCommand::acquire();

		pushGraphicsCommand(command);
	}

	void GraphicsContext::clearDefRenderTarget(const float clearValue[4])
	{
		auto* const command = ClearDefRenderTargetCommand::acquire();

		command->set(clearValue);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::clearRenderTarget(const Resource::RenderTargetDesc& desc, const float clearValue[4])
	{
		auto* const command = ClearRenderTargetCommand::acquire();

		command->set(desc, clearValue);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::clearRenderTargetInstant(const Resource::RenderTargetDesc& desc,
	                                               const float clearValue[4])
	{
		auto* const command = ClearRenderTargetInstantCommand::acquire();

		command->set(desc, clearValue);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::clearDepthStencil(const D3D12_CLEAR_FLAGS flags,
	                                        const float depth,
	                                        const uint8_t stencil)
	{
		auto* const command = ClearDepthStencilCommand::acquire();

		command->set(Resource::DepthStencilDesc{}, flags, depth, stencil);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::clearDepthStencilInstant(const Resource::DepthStencilDesc& desc,
	                                               const D3D12_CLEAR_FLAGS flags,
	                                               const float depth,
	                                               const uint8_t stencil)
	{
		auto* const command = ClearDepthStencilInstantCommand::acquire();

		command->set(desc, flags, depth, stencil);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setIndexBuffer(const Resource::IndexBufferDesc& indexBuffer)
	{
		auto* const command = SetIndexBufferCommand::acquire();

		command->set(indexBuffer);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology)
	{
		auto* const command = SetPrimitiveTopologyCommand::acquire();

		command->set(topology);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setViewport(const float width, const float height)
	{
		auto* const command = SetViewportCommand::acquire();

		command->set(width, height);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setViewport(const uint32_t width, const uint32_t height)
	{
		setViewport(static_cast<float>(width), static_cast<float>(height));
	}

	void GraphicsContext::setViewport(const DirectX::XMUINT2 dimension)
	{
		setViewport(dimension.x, dimension.y);
	}

	void GraphicsContext::setScissorRect(const int32_t left,
	                                     const int32_t top,
	                                     const int32_t right,
	                                     const int32_t bottom)
	{
		auto* const command = SetScissorRectCommand::acquire();

		command->set(left, top, right, bottom);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::setScissorRect(const float left,
	                                     const float top,
	                                     const float right,
	                                     const float bottom)
	{
		setScissorRect(static_cast<int32_t>(left),
		               static_cast<int32_t>(top),
		               static_cast<int32_t>(right),
		               static_cast<int32_t>(bottom));
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
		Resource::ResourceBase* const resource = desc.resource;

		if (resource == nullptr)
		{
			THROWLOG(LOGERROR() << "资源不能为空");
		}

		if (!resource->getPersistent())
		{
			graphicsCommandExecuter->addNonPersistentResource(resource);
		}

		auto* const command = ClearUnorderedAccessFloatCommand::acquire();

		command->set(desc, values);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::clearUnorderedAccess(const Resource::UAVClearDesc& desc, const uint32_t values[4])
	{
		Resource::ResourceBase* const resource = desc.resource;

		if (resource == nullptr)
		{
			THROWLOG(LOGERROR() << "资源不能为空");
		}

		if (!resource->getPersistent())
		{
			graphicsCommandExecuter->addNonPersistentResource(resource);
		}

		auto* const command = ClearUnorderedAccessUintCommand::acquire();

		command->set(desc, values);

		pushGraphicsCommand(command);
	}

	void GraphicsContext::draw(const uint32_t vertexCountPerInstance,
	                           const uint32_t instanceCount,
	                           const uint32_t startVertexLocation,
	                           const uint32_t startInstanceLocation)
	{
		flushShaderConstants<D3D12Core::ShaderType::VERTEX>();

		flushShaderConstants<D3D12Core::ShaderType::HULL>();

		flushShaderConstants<D3D12Core::ShaderType::DOMAIN>();

		flushShaderConstants<D3D12Core::ShaderType::GEOMETRY>();

		flushShaderConstants<D3D12Core::ShaderType::PIXEL>();

		flushShaderConstants<D3D12Core::ShaderType::AMPLIFICATION>();

		flushShaderConstants<D3D12Core::ShaderType::MESH>();

		if (setConstantsCommands.size())
		{
			//将常量设置指令重排序到SetPipelineState后
			for (auto* const command : setConstantsCommands)
			{
				pushGraphicsCommand(command);
			}

			setConstantsCommands.clear();
		}

		const uint32_t numPixelShaderResourceDesc =
			static_cast<uint32_t>(graphicsCommandExecuter->getPixelShaderResourceDescsWriteLocation() -
		                          pixelShaderResourceDescPtrOffset);

		const uint32_t numNonPixelShaderResourceDesc =
			static_cast<uint32_t>(graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation() -
		                          nonPixelShaderResourceDescPtrOffset);

		auto* const command = DrawCommand::acquire();

		command->set(pixelShaderResourceDescPtrOffset,
		             numPixelShaderResourceDesc,
		             nonPixelShaderResourceDescPtrOffset,
		             numNonPixelShaderResourceDesc,
		             vertexCountPerInstance,
		             instanceCount,
		             startVertexLocation,
		             startInstanceLocation);

		pushGraphicsCommand(command);

		pixelShaderResourceDescPtrOffset =
			graphicsCommandExecuter->getPixelShaderResourceDescsWriteLocation();

		nonPixelShaderResourceDescPtrOffset =
			graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation();
	}

	void GraphicsContext::drawQuad()
	{
		draw(3, 1, 0, 0);
	}

	void GraphicsContext::drawIndexed(const uint32_t indexCountPerInstance,
	                                  const uint32_t instanceCount,
	                                  const uint32_t startIndexLocation,
	                                  const int32_t baseVertexLocation,
	                                  const uint32_t startInstanceLocation)
	{
		flushShaderConstants<D3D12Core::ShaderType::VERTEX>();

		flushShaderConstants<D3D12Core::ShaderType::HULL>();

		flushShaderConstants<D3D12Core::ShaderType::DOMAIN>();

		flushShaderConstants<D3D12Core::ShaderType::GEOMETRY>();

		flushShaderConstants<D3D12Core::ShaderType::PIXEL>();

		flushShaderConstants<D3D12Core::ShaderType::AMPLIFICATION>();

		flushShaderConstants<D3D12Core::ShaderType::MESH>();

		if (setConstantsCommands.size())
		{
			//将常量设置指令重排序到SetPipelineState后
			for (auto* const command : setConstantsCommands)
			{
				pushGraphicsCommand(command);
			}

			setConstantsCommands.clear();
		}

		const uint32_t numPixelShaderResourceDesc =
			static_cast<uint32_t>(graphicsCommandExecuter->getPixelShaderResourceDescsWriteLocation() -
		                          pixelShaderResourceDescPtrOffset);

		const uint32_t numNonPixelShaderResourceDesc =
			static_cast<uint32_t>(graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation() -
		                          nonPixelShaderResourceDescPtrOffset);

		auto* const command = DrawIndexedCommand::acquire();

		command->set(pixelShaderResourceDescPtrOffset,
		             numPixelShaderResourceDesc,
		             nonPixelShaderResourceDescPtrOffset,
		             numNonPixelShaderResourceDesc,
		             indexCountPerInstance,
		             instanceCount,
		             startIndexLocation,
		             baseVertexLocation,
		             startInstanceLocation);

		pushGraphicsCommand(command);

		pixelShaderResourceDescPtrOffset =
			graphicsCommandExecuter->getPixelShaderResourceDescsWriteLocation();

		nonPixelShaderResourceDescPtrOffset =
			graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation();
	}

	void GraphicsContext::dispatchGrp(const uint32_t threadGroupCountX,
	                                  const uint32_t threadGroupCountY,
	                                  const uint32_t threadGroupCountZ)
	{
		flushShaderConstants<D3D12Core::ShaderType::COMPUTE>();

		if (setConstantsCommands.size())
		{
			//将常量设置指令重排序到SetPipelineState后
			for (auto* const command : setConstantsCommands)
			{
				pushGraphicsCommand(command);
			}

			setConstantsCommands.clear();
		}

		const uint32_t numNonPixelShaderResourceDesc =
			static_cast<uint32_t>(graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation() -
		                          nonPixelShaderResourceDescPtrOffset);

		auto* const command = DispatchGrpCommand::acquire();

		command->set(nonPixelShaderResourceDescPtrOffset,
		             numNonPixelShaderResourceDesc,
		             threadGroupCountX,
		             threadGroupCountY,
		             threadGroupCountZ);

		pushGraphicsCommand(command);

		nonPixelShaderResourceDescPtrOffset =
			graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation();
	}

	void GraphicsContext::dispatchDim(const uint32_t dispatchThreadCountX,
	                                  const uint32_t dispatchThreadCountY,
	                                  const uint32_t dispatchThreadCountZ)
	{
		flushShaderConstants<D3D12Core::ShaderType::COMPUTE>();

		if (setConstantsCommands.size())
		{
			//将常量设置指令重排序到SetPipelineState后
			for (auto* const command : setConstantsCommands)
			{
				pushGraphicsCommand(command);
			}

			setConstantsCommands.clear();
		}

		const uint32_t numNonPixelShaderResourceDesc =
			static_cast<uint32_t>(graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation() -
		                          nonPixelShaderResourceDescPtrOffset);

		auto* const command = DispatchDimCommand::acquire();

		command->set(nonPixelShaderResourceDescPtrOffset,
		             numNonPixelShaderResourceDesc,
		             dispatchThreadCountX,
		             dispatchThreadCountY,
		             dispatchThreadCountZ);

		pushGraphicsCommand(command);

		nonPixelShaderResourceDescPtrOffset =
			graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation();
	}

	void GraphicsContext::dispatchDim(const DirectX::XMUINT3 dispatchThreadCount)
	{
		dispatchDim(dispatchThreadCount.x, dispatchThreadCount.y, dispatchThreadCount.z);
	}

	void GraphicsContext::pushGraphicsCommand(GraphicsCommand* command)
	{
		graphicsCommands.push_back(command);
	}

	void GraphicsContext::begin()
	{
		graphicsCommandExecuter->begin();

#ifdef _DEBUG
		if (graphicsCommands.size())
		{
			THROWLOG(LOGERROR()
			         << "侦测到" << COLORIZEVAR(graphicsCommands)
			         << "非空，是否忘记调用 end 方法又或者是在调用 begin 方法前调用了图形上下文的方法？");
		}

		if (setConstantsCommands.size())
		{
			THROWLOG(LOGERROR() << "侦测到" << COLORIZEVAR(setConstantsCommands)
			                    << "非空，是否忘记调用 draw/dispatch 命令？");
		}
#endif // _DEBUG

		pixelShaderResourceDescPtrOffset =
			graphicsCommandExecuter->getPixelShaderResourceDescsWriteLocation();

		nonPixelShaderResourceDescPtrOffset =
			graphicsCommandExecuter->getNonPixelShaderResourceDescsWriteLocation();

		renderTargetDescsPtrOffset = graphicsCommandExecuter->getRenderTargetDescsWriteLocation();

		vertexBufferDescsPtrOffset = graphicsCommandExecuter->getVertexBufferDescsWriteLocation();

		for (uint32_t i = 0; i < static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT); i++)
		{
			shaderConstantsPtrOffsets[i] = graphicsCommandExecuter->getShaderConstantsWriteLocation(
				static_cast<D3D12Core::ShaderType>(i));

			shaderConstantsStartOffsets[i] = UINT32_MAX;

			numShaderConstantsValues[i] = 0;
		}

		CommandPoolRegistry::resetAll();
	}

	void GraphicsContext::end()
	{
		//拷贝描述符到全局唯一着色器可见堆，从而获得描述符索引
		graphicsCommandExecuter->copyDescriptors();

		//获得描述符索引后解析描述符索引
		graphicsCommandExecuter->resolveDescriptorIndices();

		//执行所有命令
		if (graphicsCommands.size())
		{
			for (auto* const command : graphicsCommands)
			{
				command->execute(*graphicsCommandExecuter);
			}

			graphicsCommands.clear();
		}
	}

	D3D12Core::GraphicsCommandList* GraphicsContext::getCommandList() const
	{
		return graphicsCommandExecuter->getCommandList();
	}

}

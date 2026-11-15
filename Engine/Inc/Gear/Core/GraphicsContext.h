#pragma once

#ifndef _GEAR_CORE_GRAPHICSCONTEXT_H_
#define _GEAR_CORE_GRAPHICSCONTEXT_H_

#include "GraphicsCommand/GraphicsCommandExecuter.h"

#include <Gear/Core/GraphicsCommand/GraphicsCommand.h>

#include <Gear/Core/GraphicsCommand/LambdaCommand.h>

#include <Gear/Core/GraphicsCommand/SetRenderTargetsCommandBase.h>

#include <Gear/Core/GraphicsCommand/SetVertexBuffersCommand.h>

#include <Gear/Core/GraphicsCommand/SetShaderConstantBufferCommand.h>

#include <Gear/Core/GraphicsCommand/SetShaderConstantsCommand.h>

//创建一个作用域，内置默认值为0的临时偏移co，它会随着API调用自增，建议搭配CodeSnippets中的片段使用
//注意：作用域内只允许ctx->set*SConstants调用！因为宏定义是无法处理某些特殊情况的
#define SETCONSTS(...) \
	{ \
		uint32_t co = 0u; \
		__VA_ARGS__ \
	}

namespace Gear::Core
{
	namespace CLEARFLAG
	{
		constexpr D3D12_CLEAR_FLAGS DEPTH = D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH;

		constexpr D3D12_CLEAR_FLAGS STENCIL = D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL;

		constexpr D3D12_CLEAR_FLAGS ALL =
			D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL;
	}

	CREATESAFETYPE(GraphicsContext);

	/// <summary>
	/// 注意：禁止将UAV用于只读用途，否则引擎可能会插入多余的UAV屏障
	/// </summary>
	class GraphicsContext
	{
	public:

		GraphicsContext(const GraphicsContext&) = delete;

		void operator=(const GraphicsContext&) = delete;

		GraphicsContext();

		~GraphicsContext();

		//更新高级缓冲对象
		void updateBuffer(Resource::BufferView& bufferView, const void* const data, const uint32_t size);

		//更新默认常量缓冲
		void updateBuffer(Resource::DefaultCBuffer& defaultCBuffer,
		                  const void* const data,
		                  const uint32_t size);

		//以下的方法用于设置每个着色器独享的根常量

		template<size_t N>
		void setVSConstants(const Resource::ShaderResourceDesc (&descs)[N], uint32_t& offset);

		template<size_t N>
		void setHSConstants(const Resource::ShaderResourceDesc (&descs)[N], uint32_t& offset);

		template<size_t N>
		void setDSConstants(const Resource::ShaderResourceDesc (&descs)[N], uint32_t& offset);

		template<size_t N>
		void setGSConstants(const Resource::ShaderResourceDesc (&descs)[N], uint32_t& offset);

		template<size_t N>
		void setPSConstants(const Resource::ShaderResourceDesc (&descs)[N], uint32_t& offset);

		template<size_t N>
		void setCSConstants(const Resource::ShaderResourceDesc (&descs)[N], uint32_t& offset);

		template<typename StructType>
			requires std::is_class_v<StructType>
		void setVSConstants(const StructType& structVal, uint32_t& offset);

		template<typename StructType>
			requires std::is_class_v<StructType>
		void setHSConstants(const StructType& structVal, uint32_t& offset);

		template<typename StructType>
			requires std::is_class_v<StructType>
		void setDSConstants(const StructType& structVal, uint32_t& offset);

		template<typename StructType>
			requires std::is_class_v<StructType>
		void setGSConstants(const StructType& structVal, uint32_t& offset);

		template<typename StructType>
			requires std::is_class_v<StructType>
		void setPSConstants(const StructType& structVal, uint32_t& offset);

		template<typename StructType>
			requires std::is_class_v<StructType>
		void setCSConstants(const StructType& structVal, uint32_t& offset);

		void setVSConstants(const uint32_t numValues, const void* const data, uint32_t& offset);

		void setHSConstants(const uint32_t numValues, const void* const data, uint32_t& offset);

		void setDSConstants(const uint32_t numValues, const void* const data, uint32_t& offset);

		void setGSConstants(const uint32_t numValues, const void* const data, uint32_t& offset);

		void setPSConstants(const uint32_t numValues, const void* const data, uint32_t& offset);

		void setCSConstants(const uint32_t numValues, const void* const data, uint32_t& offset);

		//设置每个着色器共享的常量缓冲
		void setGlobalConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		//以下的方法用于设置每个着色器独享的常量缓冲

		void setVSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setHSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setDSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setGSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setPSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setCSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setPipelineState(D3D12Core::GraphicsState& graphicsPipelineState);

		void setPipelineState(D3D12Core::ComputeState& computePipelineState);

		//以下的方法用于设置渲染目标

		template<size_t N>
		void setRenderTargets(const Resource::RenderTargetDesc (&renderTargets)[N],
		                      const Resource::DepthStencilDesc& depthStencil = {});

		void setRenderTargets(const Resource::DepthStencilDesc& depthStencil);

		//将后备缓冲设置为渲染目标
		void setDefRenderTarget();

		//清理后备缓冲
		void clearDefRenderTarget(const float clearValue[4]);

		//推迟到draw call清理渲染目标视图
		void clearRenderTarget(const Resource::RenderTargetDesc& desc, const float clearValue[4]);

		//立刻清理渲染目标视图
		void clearRenderTargetInstant(const Resource::RenderTargetDesc& desc, const float clearValue[4]);

		//清理下一个draw call即将绑定或已经绑定的深度模板视图
		void clearDepthStencil(const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil);

		//立刻清理深度模板视图
		void clearDepthStencilInstant(const Resource::DepthStencilDesc& desc,
		                              const D3D12_CLEAR_FLAGS flags,
		                              const float depth,
		                              const uint8_t stencil);

		//设置顶点缓冲
		template<size_t N>
		void setVertexBuffers(const Resource::VertexBufferDesc (&vertexBuffers)[N], const uint32_t startSlot);

		//设置索引缓冲
		void setIndexBuffer(const Resource::IndexBufferDesc& indexBuffers);

		//设置图元拓扑
		void setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology);

		//设置视口
		void setViewport(const float width, const float height);

		//设置视口
		void setViewport(const uint32_t width, const uint32_t height);

		//设置视口
		void setViewport(const DirectX::XMUINT2 dimension);

		//设置剪刀矩形
		void setScissorRect(const int32_t left, const int32_t top, const int32_t right, const int32_t bottom);

		//设置剪刀矩形
		void setScissorRect(const float left, const float top, const float right, const float bottom);

		//以下是帮手方法，同时设置视口和剪刀矩形

		void setViewportSimple(const float width, const float height);

		void setViewportSimple(const uint32_t width, const uint32_t height);

		void setViewportSimple(const DirectX::XMUINT2 dimension);

		//清理UAV
		void clearUnorderedAccess(const Resource::UAVClearDesc& desc, const float values[4]);

		//清理UAV
		void clearUnorderedAccess(const Resource::UAVClearDesc& desc, const uint32_t values[4]);

		//绘制实例
		void draw(const uint32_t vertexCountPerInstance,
		          const uint32_t instanceCount,
		          const uint32_t startVertexLocation,
		          const uint32_t startInstanceLocation);

		//绘制全屏三角形
		void drawQuad();

		//绘制索引化实例
		void drawIndexed(const uint32_t indexCountPerInstance,
		                 const uint32_t instanceCount,
		                 const uint32_t startIndexLocation,
		                 const int32_t baseVertexLocation,
		                 const uint32_t startInstanceLocation);

		//分发线程（设置线程组维度）
		void dispatchGrp(const uint32_t threadGroupCountX,
		                 const uint32_t threadGroupCountY,
		                 const uint32_t threadGroupCountZ);

		//分发线程（设置线程维度）
		void dispatchDim(const uint32_t dispatchThreadCountX,
		                 const uint32_t dispatchThreadCountY,
		                 const uint32_t dispatchThreadCountZ);

		//分发线程（设置线程维度）
		void dispatchDim(const DirectX::XMUINT3 dispatchThreadCount);

		//推送自定义图形命令
		void pushGraphicsCommand(GraphicsCommand* command);

		//由RenderTask自动调用
		void begin();

		//由RenderTask和RenderThread自动调用
		void end();

		D3D12Core::GraphicsCommandList* getCommandList() const;

	private:

		//从提供的ShaderResourceDesc提取索引到shaderConstants
		template<D3D12Core::ShaderType ShaderType, size_t N>
		void setShaderConstants(const Resource::ShaderResourceDesc (&descs)[N], uint32_t& offset);

		template<D3D12Core::ShaderType ShaderType>
		void setShaderConstants(const uint32_t numValues, const void* const data, uint32_t& offset);

		template<D3D12Core::ShaderType ShaderType>
		void flushShaderConstants();

		GraphicsCommandExecuterPtr graphicsCommandExecuter;

		std::vector<GraphicsCommand*> graphicsCommands;

		//需要进行重排序，用户可能在任意位置调用 SetPipelineState，而 SetPipelineState 可能改变根签名，因此需要暂存并将其重排序到仅在 Draw/Dispatch Call 之前
		//最多有 D3D12Core::RootSignature::maxPerShaderConstants * D3D12Core::ShaderType::TYPECOUNT 个设置常量命令
		Utils::StaticVector<GraphicsCommand*,
		                    D3D12Core::RootSignature::maxPerShaderConstants* static_cast<uint32_t>(
								D3D12Core::ShaderType::TYPECOUNT)>
			setConstantsCommands;

		//以下是用于每次draw call或dispatch call的临时资源

		uint64_t pixelShaderResourceDescPtrOffset;

		uint64_t nonPixelShaderResourceDescPtrOffset;

		uint64_t renderTargetDescsPtrOffset;

		uint64_t vertexBufferDescsPtrOffset;

		uint64_t shaderConstantsPtrOffsets[static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT)];

		std::array<uint32_t, D3D12Core::RootSignature::maxPerShaderConstants>
			shaderConstants[static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT)];

		uint32_t numShaderConstantsValues[static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT)];

		uint32_t shaderConstantsStartOffsets[static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT)];
	};

	template<size_t N>
	inline void GraphicsContext::setVSConstants(const Resource::ShaderResourceDesc (&descs)[N],
	                                            uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::VERTEX>(descs, offset);
	}

	template<size_t N>
	inline void GraphicsContext::setHSConstants(const Resource::ShaderResourceDesc (&descs)[N],
	                                            uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::HULL>(descs, offset);
	}

	template<size_t N>
	inline void GraphicsContext::setDSConstants(const Resource::ShaderResourceDesc (&descs)[N],
	                                            uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::DOMAIN>(descs, offset);
	}

	template<size_t N>
	inline void GraphicsContext::setGSConstants(const Resource::ShaderResourceDesc (&descs)[N],
	                                            uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::GEOMETRY>(descs, offset);
	}

	template<size_t N>
	inline void GraphicsContext::setPSConstants(const Resource::ShaderResourceDesc (&descs)[N],
	                                            uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::PIXEL>(descs, offset);
	}

	template<size_t N>
	inline void GraphicsContext::setCSConstants(const Resource::ShaderResourceDesc (&descs)[N],
	                                            uint32_t& offset)
	{
		setShaderConstants<D3D12Core::ShaderType::COMPUTE>(descs, offset);
	}

	template<typename StructType>
		requires std::is_class_v<StructType>
	inline void GraphicsContext::setVSConstants(const StructType& structVal, uint32_t& offset)
	{
		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setVSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
		requires std::is_class_v<StructType>
	inline void GraphicsContext::setHSConstants(const StructType& structVal, uint32_t& offset)
	{
		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setHSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
		requires std::is_class_v<StructType>
	inline void GraphicsContext::setDSConstants(const StructType& structVal, uint32_t& offset)
	{
		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setDSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
		requires std::is_class_v<StructType>
	inline void GraphicsContext::setGSConstants(const StructType& structVal, uint32_t& offset)
	{
		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setGSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
		requires std::is_class_v<StructType>
	inline void GraphicsContext::setPSConstants(const StructType& structVal, uint32_t& offset)
	{
		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setPSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
		requires std::is_class_v<StructType>
	inline void GraphicsContext::setCSConstants(const StructType& structVal, uint32_t& offset)
	{
		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setCSConstants(numElements, &structVal, offset);
	}

	template<D3D12Core::ShaderType ShaderType, size_t N>
	inline void GraphicsContext::setShaderConstants(const Resource::ShaderResourceDesc (&descs)[N],
	                                                uint32_t& offset)
	{
#ifdef _DEBUG
		if (N == 0)
		{
			THROWLOG(LOGERROR() << "不可写入" << 0 << "个常量");
		}
#endif

		constexpr uint32_t shaderIndex = static_cast<uint32_t>(ShaderType);

		if (shaderConstantsStartOffsets[shaderIndex] == UINT32_MAX)
		{
			shaderConstantsStartOffsets[shaderIndex] = offset;
		}

		const uint32_t expectedOffset =
			shaderConstantsStartOffsets[shaderIndex] + numShaderConstantsValues[shaderIndex];

		//使用expectedOffset来检测是否有非连续写入
		//如有则结束当前的批次写入
		if (expectedOffset != offset)
		{
			flushShaderConstants<ShaderType>();

			setShaderConstants<ShaderType>(descs, offset);
		}
		else
		{
#ifdef _DEBUG
			if (offset + N > D3D12Core::RootSignature::maxPerShaderConstants)
			{
				THROWLOG(LOGERROR() << "侦测到写入终止位置" << (offset + N - 1) << "超出临时缓冲区终止位置"
				                    << (D3D12Core::RootSignature::maxPerShaderConstants - 1));
			}
#endif

			for (uint32_t i = 0; i < N; i++)
			{
				if constexpr (ShaderType == D3D12Core::ShaderType::PIXEL)
				{
					graphicsCommandExecuter->pushPixelShaderResourceDesc(descs[i]);
				}
				else
				{
					graphicsCommandExecuter->pushNonPixelShaderResourceDesc(descs[i]);
				}

				Resource::ResourceBase* const resource = descs[i].resource;

				if (resource && !resource->getPersistent())
				{
					graphicsCommandExecuter->addNonPersistentResource(resource);

					shaderConstants[shaderIndex][offset + i] = UINT32_MAX;

					const uint64_t unsettedLocation = shaderConstantsPtrOffsets[shaderIndex] + i + offset -
					                                  shaderConstantsStartOffsets[shaderIndex];

					graphicsCommandExecuter->addUnsettedIndex(
						ShaderType, descs[i].resourceIndex, unsettedLocation);
				}
				else
				{
					shaderConstants[shaderIndex][offset + i] = *(descs[i].resourceIndex);
				}
			}

			offset += N;

			numShaderConstantsValues[shaderIndex] += N;
		}
	}

	template<D3D12Core::ShaderType ShaderType>
	void GraphicsContext::setShaderConstants(const uint32_t numValues,
	                                         const void* const data,
	                                         uint32_t& offset)
	{
#ifdef _DEBUG
		if (numValues == 0)
		{
			THROWLOG(LOGERROR() << "不可写入" << 0 << "个常量");
		}

		if (data == nullptr)
		{
			THROWLOG(LOGERROR() << "传入的数据指针不能为 nullptr");
		}
#endif

		constexpr uint32_t shaderIndex = static_cast<uint32_t>(ShaderType);

		if (shaderConstantsStartOffsets[shaderIndex] == UINT32_MAX)
		{
			shaderConstantsStartOffsets[shaderIndex] = offset;
		}

		const uint32_t expectedOffset =
			shaderConstantsStartOffsets[shaderIndex] + numShaderConstantsValues[shaderIndex];

		//使用expectedOffset来检测是否有非连续写入
		//如有则结束当前的批次写入
		if (expectedOffset != offset)
		{
			flushShaderConstants<ShaderType>();

			setShaderConstants<ShaderType>(numValues, data, offset);
		}
		else
		{
#ifdef _DEBUG
			if (offset + numValues > D3D12Core::RootSignature::maxPerShaderConstants)
			{
				THROWLOG(LOGERROR() << "侦测到写入终止位置" << (offset + numValues - 1)
				                    << "超出临时缓冲区终止位置"
				                    << (D3D12Core::RootSignature::maxPerShaderConstants - 1));
			}
#endif

			memcpy(shaderConstants[shaderIndex].data() + offset, data, numValues * sizeof(uint32_t));

			offset += numValues;

			numShaderConstantsValues[shaderIndex] += numValues;
		}
	}

	template<D3D12Core::ShaderType ShaderType>
	void GraphicsContext::flushShaderConstants()
	{
		constexpr uint32_t shaderIndex = static_cast<uint32_t>(ShaderType);

		if (numShaderConstantsValues[shaderIndex] == 0)
		{
			shaderConstantsStartOffsets[shaderIndex] = UINT32_MAX;

			return;
		}

		const uint64_t dataPtrOffset = shaderConstantsPtrOffsets[shaderIndex];

		const uint32_t numConstantValues = numShaderConstantsValues[shaderIndex];

		const uint32_t constantWriteOffset = shaderConstantsStartOffsets[shaderIndex];

		graphicsCommandExecuter->pushShaderConstants(ShaderType,
		                                             shaderConstants[shaderIndex].data() +
		                                                 shaderConstantsStartOffsets[shaderIndex],
		                                             numShaderConstantsValues[shaderIndex]);

		shaderConstantsPtrOffsets[shaderIndex] =
			graphicsCommandExecuter->getShaderConstantsWriteLocation(ShaderType);

		if constexpr (ShaderType == D3D12Core::ShaderType::VERTEX)
		{
			auto* const command = SetVSConstantsCommand::acquire();

			command->set(dataPtrOffset, numConstantValues, constantWriteOffset);

			setConstantsCommands.push(command);
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::HULL)
		{
			auto* const command = SetHSConstantsCommand::acquire();

			command->set(dataPtrOffset, numConstantValues, constantWriteOffset);

			setConstantsCommands.push(command);
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::DOMAIN)
		{
			auto* const command = SetDSConstantsCommand::acquire();

			command->set(dataPtrOffset, numConstantValues, constantWriteOffset);

			setConstantsCommands.push(command);
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::GEOMETRY)
		{
			auto* const command = SetGSConstantsCommand::acquire();

			command->set(dataPtrOffset, numConstantValues, constantWriteOffset);

			setConstantsCommands.push(command);
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::PIXEL)
		{
			auto* const command = SetPSConstantsCommand::acquire();

			command->set(dataPtrOffset, numConstantValues, constantWriteOffset);

			setConstantsCommands.push(command);
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::COMPUTE)
		{
			auto* const command = SetCSConstantsCommand::acquire();

			command->set(dataPtrOffset, numConstantValues, constantWriteOffset);

			setConstantsCommands.push(command);
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::AMPLIFICATION)
		{
			auto* const command = SetASConstantsCommand::acquire();

			command->set(dataPtrOffset, numConstantValues, constantWriteOffset);

			setConstantsCommands.push(command);
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::MESH)
		{
			auto* const command = SetMSConstantsCommand::acquire();

			command->set(dataPtrOffset, numConstantValues, constantWriteOffset);

			setConstantsCommands.push(command);
		}

		shaderConstantsStartOffsets[shaderIndex] = UINT32_MAX;

		numShaderConstantsValues[shaderIndex] = 0;
	}

	template<size_t N>
	inline void GraphicsContext::setRenderTargets(const Resource::RenderTargetDesc (&renderTargets)[N],
	                                              const Resource::DepthStencilDesc& depthStencil)
	{
#ifdef _DEBUG
		if (N > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
		{
			THROWLOG(LOGERROR() << "无法绑定超过" << D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT << "个渲染目标");
		}
#endif // _DEBUG

		for (uint32_t i = 0; i < N; i++)
		{
			graphicsCommandExecuter->pushRenderTargetDesc(renderTargets[i]);
		}

		auto* const command = SetRenderTargetsCommand::acquire();

		command->set(renderTargetDescsPtrOffset, N, depthStencil);

		pushGraphicsCommand(command);

		renderTargetDescsPtrOffset = graphicsCommandExecuter->getRenderTargetDescsWriteLocation();
	}

	template<size_t N>
	inline void GraphicsContext::setVertexBuffers(const Resource::VertexBufferDesc (&vertexBuffers)[N],
	                                              const uint32_t startSlot)
	{
#ifdef _DEBUG
		if (startSlot + N > D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT)
		{
			THROWLOG(LOGERROR() << "无法绑定超过" << D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT
			                    << "个顶点缓冲");
		}
#endif // _DEBUG

		for (uint32_t i = 0; i < N; i++)
		{
			graphicsCommandExecuter->pushVertexBufferDesc(vertexBuffers[i]);
		}

		auto* const command = SetVertexBuffersCommand::acquire();

		command->set(vertexBufferDescsPtrOffset, N, startSlot);

		pushGraphicsCommand(command);

		vertexBufferDescsPtrOffset = graphicsCommandExecuter->getVertexBufferDescsWriteLocation();
	}
}

#endif // !_GEAR_CORE_GRAPHICSCONTEXT_H_

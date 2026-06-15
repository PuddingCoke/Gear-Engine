#pragma once

#ifndef _GEAR_CORE_GRAPHICSCONTEXT_H_
#define _GEAR_CORE_GRAPHICSCONTEXT_H_

#include<Gear/Core/D3D12Core/CommandList.h>

#include<Gear/Core/D3D12Core/RootSignature.h>

#include<Gear/Core/D3D12Core/PipelineState.h>

#include<Gear/Core/Resource/BufferView.h>

#include<Gear/Core/Resource/DefaultCBuffer.h>

#include<Gear/Utils/StaticVector.h>

#include<array>

#include<type_traits>

#include"TOPOLOGY.h"

//创建一个作用域，内置默认值为0的临时偏移co，它会随着API调用自增，建议搭配CodeSnippets中的片段使用
//注意事项：作用域内只允许ctx->set*SConstants调用！因为宏定义是无法处理某些特殊情况的
#define SETCONSTS(...) { uint32_t co = 0u; __VA_ARGS__ }

namespace Gear::Core
{
	namespace CLEARFLAG
	{
		constexpr D3D12_CLEAR_FLAGS DEPTH = D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH;

		constexpr D3D12_CLEAR_FLAGS STENCIL = D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL;

		constexpr D3D12_CLEAR_FLAGS ALL = D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL;
	}

	CREATESAFETYPE(GraphicsContext);

	/// <summary>
	/// 注意事项：禁止将UAV用于只读用途，否则引擎可能会插入多余的UAV屏障
	/// </summary>
	class GraphicsContext
	{
	public:

		GraphicsContext(const GraphicsContext&) = delete;

		void operator=(const GraphicsContext&) = delete;

		GraphicsContext();

		~GraphicsContext();

		//更新高级缓冲对象
		void updateBuffer(Resource::BufferView& bufferView, const void* const data, const uint32_t size) const;

		//更新静态常量缓冲
		void updateBuffer(Resource::DefaultCBuffer& staticCBuffer, const void* const data, const uint32_t size) const;

		//设置用户自定义的全局常量缓冲
		void setGlobalConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		//以下的方法用于设置每个着色器独享的根常量

		template<size_t N>
		void setVSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset);

		template<size_t N>
		void setHSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset);

		template<size_t N>
		void setDSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset);

		template<size_t N>
		void setGSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset);

		template<size_t N>
		void setPSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset);

		template<size_t N>
		void setCSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset);

		template<typename StructType>
		void setVSConstants(const StructType& structVal, uint32_t& offset) const;

		template<typename StructType>
		void setHSConstants(const StructType& structVal, uint32_t& offset) const;

		template<typename StructType>
		void setDSConstants(const StructType& structVal, uint32_t& offset) const;

		template<typename StructType>
		void setGSConstants(const StructType& structVal, uint32_t& offset) const;

		template<typename StructType>
		void setPSConstants(const StructType& structVal, uint32_t& offset) const;

		template<typename StructType>
		void setCSConstants(const StructType& structVal, uint32_t& offset) const;

		void constantsWriteCheck(const D3D12Core::RootSignature::ShaderType shaderType, const uint32_t numWrite) const;

		void setVSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setHSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setDSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setGSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setPSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setCSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		//以下的方法用于设置每个着色器独享的常量缓冲

		void setVSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setHSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setDSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setGSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setPSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setCSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setPipelineState(const D3D12Core::PipelineState& pipelineState);

		//以下的方法用于设置渲染目标

		template<size_t N>
		void setRenderTargets(const Resource::D3D12Resource::RenderTargetDesc(&renderTargets)[N], const Resource::D3D12Resource::DepthStencilDesc& depthStencil = {});

		void setRenderTargets(const Resource::D3D12Resource::DepthStencilDesc& depthStencil);

		//将后备缓冲设置为渲染目标
		void setDefRenderTarget() const;

		//清理后备缓冲
		void clearDefRenderTarget(const float clearValue[4]) const;

		//推迟到draw call清理渲染目标视图
		void clearRenderTarget(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4]);

		//立刻清理渲染目标视图
		void clearRenderTargetInstant(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4]);

		//清理下一个draw call即将绑定或已经绑定的深度模板视图
		void clearDepthStencil(const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil);

		//立刻清理深度模板视图
		void clearDepthStencilInstant(const Resource::D3D12Resource::DepthStencilDesc& desc, const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil);

		//设置顶点缓冲
		template<size_t N>
		void setVertexBuffers(const uint32_t startSlot, const Resource::D3D12Resource::VertexBufferDesc(&vertexBuffers)[N]);

		//设置索引缓冲
		void setIndexBuffer(const Resource::D3D12Resource::IndexBufferDesc& indexBuffers);

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
		void clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const float values[4]);

		//清理UAV
		void clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const uint32_t values[4]);

		//绘制实例
		void draw(const uint32_t vertexCountPerInstance, const uint32_t instanceCount, const uint32_t startVertexLocation, const uint32_t startInstanceLocation);

		//绘制全屏三角形
		void drawQuad();

		//绘制索引化实例
		void drawIndexed(const uint32_t indexCountPerInstance, const uint32_t instanceCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation, const uint32_t startInstanceLocation);

		//分发线程（设置线程组维度）
		void dispatchGrp(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ);

		//分发线程（设置线程维度）
		void dispatchDim(const uint32_t dispatchThreadCountX, const uint32_t dispatchThreadCountY, const uint32_t dispatchThreadCountZ);

		//分发线程（设置线程维度）
		void dispatchDim(const DirectX::XMUINT3 dispatchThreadCount);

		//由RenderTask自动调用
		void begin();

		//重置内部追踪的状态
		void resetTrackedStates();

		void resetPipelineState();

		void resetTrackedGraphicsStates();

		void resetTrackedComputeStates();
		/////////////////

		D3D12Core::CommandList* getCommandList() const;

	private:

		struct RootConstantBufferDesc
		{
			uint32_t rootParameterIndex;

			D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
		};

		struct RenderTargetClearDesc
		{
			D3D12_CPU_DESCRIPTOR_HANDLE handle;

			float clearValue[4];
		};

		struct DepthStencilClearDesc
		{
			void setHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle);

			void setClearData(const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil);

			void reset();

			D3D12_CPU_DESCRIPTOR_HANDLE handle = {};

			D3D12_CLEAR_FLAGS flags = CLEARFLAG::ALL;

			float depth = 0;

			uint8_t stencil = 0;

			bool needClear = false;
		};

		void transitionResources();

		template<size_t N>
		void setShaderResources(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t targetSRVState);

		//从提供的ShaderResourceDesc提取索引
		template<size_t N>
		void getResourceIndicesFromDescs(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N]);

		void pushRootConstantBufferDesc(const RootConstantBufferDesc& desc);

		void flushRootConstantBufferDescs(const bool isGraphicsRootSignature);

		void flushRenderTargetClearDescs();

		void resetDepthStencilClearDesc();

		void setGraphicsRootSignature(const D3D12Core::RootSignature* const rootSignature);

		void setComputeRootSignature(const D3D12Core::RootSignature* const rootSignature);

		const D3D12Core::RootSignature* getGraphicsRootSignature() const;

		const D3D12Core::RootSignature* getComputeRootSignature() const;

		//根据Desc设置对应资源的转变状态
		void setResourceState(const Resource::D3D12Resource::ShaderResourceDesc& desc, const uint32_t targetSRVState);

		void setResourceState(const Resource::D3D12Resource::RenderTargetDesc& desc);

		void setResourceState(const Resource::D3D12Resource::DepthStencilDesc& desc);

		void setResourceState(const Resource::D3D12Resource::VertexBufferDesc& desc);

		void setResourceState(const Resource::D3D12Resource::IndexBufferDesc& desc);

		Resource::D3D12Resource::D3D12ResourceBase* setResourceState(const Resource::D3D12Resource::ClearUAVDesc& desc);

		//重置内部追踪的状态
		void resetGraphicsRootSignature();

		void resetPrimitiveTopology();

		void resetComputeRootSignature();

		void resetUserDefinedGlobalConstantBuffer();

		D3D12_VIEWPORT vp;

		D3D12_RECT rt;

		D3D12Core::CommandListPtr commandList;

		//以下是内部追踪的状态，用于减少图形API的调用
		const D3D12Core::PipelineState* currentPipelineState;

		const Resource::ImmutableCBuffer* userDefinedGlobalConstantBuffer;

		D3D_PRIMITIVE_TOPOLOGY primitiveTopology;

		const D3D12Core::RootSignature* graphicsRootSignature;

		const D3D12Core::RootSignature* computeRootSignature;

		//以下是用于每次draw call或dispatch call的临时资源

		//这里有个假设，微软的官方文档说每个根签名最多有 64 个 DWORD
		//如果根签名全部都是根常量缓冲，那么最多有 64/2 = 32 个根常量缓冲
		//这个想法是我在优化 GraphicsContext 时一瞬之间想到的

		Utils::StaticVector<RootConstantBufferDesc, D3D12Core::RootSignature::maxDWORD / D3D12Core::RootSignature::perDescriptorDWORD> rootConstantBufferDescs;

		Utils::StaticVector<RenderTargetClearDesc, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> renderTargetClearDescs;

		DepthStencilClearDesc depthStencilClearDesc;

		std::array<uint32_t, D3D12Core::RootSignature::maxPerShaderConstants> transientResourceIndices;

		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> transientRTVHandles;

		std::array<D3D12_VERTEX_BUFFER_VIEW, D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> transientVBViews;

	};

	template<size_t N>
	inline void GraphicsContext::setVSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset)
	{
		getResourceIndicesFromDescs(descs);

		setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		setVSConstants(N, transientResourceIndices.data(), offset);
	}

	template<size_t N>
	inline void GraphicsContext::setHSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset)
	{
		getResourceIndicesFromDescs(descs);

		setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		setHSConstants(N, transientResourceIndices.data(), offset);
	}

	template<size_t N>
	inline void GraphicsContext::setDSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset)
	{
		getResourceIndicesFromDescs(descs);

		setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		setDSConstants(N, transientResourceIndices.data(), offset);
	}

	template<size_t N>
	inline void GraphicsContext::setGSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset)
	{
		getResourceIndicesFromDescs(descs);

		setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		setGSConstants(N, transientResourceIndices.data(), offset);
	}

	template<size_t N>
	inline void GraphicsContext::setPSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset)
	{
		getResourceIndicesFromDescs(descs);

		setShaderResources(descs, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		setPSConstants(N, transientResourceIndices.data(), offset);
	}

	template<size_t N>
	inline void GraphicsContext::setCSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], uint32_t& offset)
	{
		getResourceIndicesFromDescs(descs);

		setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		setCSConstants(N, transientResourceIndices.data(), offset);
	}

	template<typename StructType>
	inline void GraphicsContext::setVSConstants(const StructType& structVal, uint32_t& offset) const
	{
		static_assert(std::is_class<StructType>::value, "StructType must be a struct/class type");

		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setVSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
	inline void GraphicsContext::setHSConstants(const StructType& structVal, uint32_t& offset) const
	{
		static_assert(std::is_class<StructType>::value, "StructType must be a struct/class type");

		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setHSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
	inline void GraphicsContext::setDSConstants(const StructType& structVal, uint32_t& offset) const
	{
		static_assert(std::is_class<StructType>::value, "StructType must be a struct/class type");

		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setDSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
	inline void GraphicsContext::setGSConstants(const StructType& structVal, uint32_t& offset) const
	{
		static_assert(std::is_class<StructType>::value, "StructType must be a struct/class type");

		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setGSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
	inline void GraphicsContext::setPSConstants(const StructType& structVal, uint32_t& offset) const
	{
		static_assert(std::is_class<StructType>::value, "StructType must be a struct/class type");

		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setPSConstants(numElements, &structVal, offset);
	}

	template<typename StructType>
	inline void GraphicsContext::setCSConstants(const StructType& structVal, uint32_t& offset) const
	{
		static_assert(std::is_class<StructType>::value, "StructType must be a struct/class type");

		const uint32_t numElements = sizeof(StructType) / sizeof(uint32_t);

		setCSConstants(numElements, &structVal, offset);
	}

	template<size_t N>
	inline void GraphicsContext::getResourceIndicesFromDescs(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N])
	{
#ifdef _DEBUG
		if (N > D3D12Core::RootSignature::maxPerShaderConstants)
		{
			LOGERROR(L"对于每个draw call和dispatch call来说，每个着色器的常量写入限制为", D3D12Core::RootSignature::maxPerShaderConstants, L"个！");
		}
#endif // _DEBUG

		for (uint32_t i = 0; i < N; i++)
		{
			transientResourceIndices[i] = descs[i].resourceIndex;
		}
	}

	template<size_t N>
	inline void GraphicsContext::setRenderTargets(const Resource::D3D12Resource::RenderTargetDesc(&renderTargets)[N], const Resource::D3D12Resource::DepthStencilDesc& depthStencil)
	{
#ifdef _DEBUG
		if (N > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
		{
			LOGERROR(L"无法绑定更多的渲染目标");
		}
#endif // _DEBUG

		for (uint32_t i = 0; i < N; i++)
		{
			transientRTVHandles[i] = renderTargets[i].rtvHandle;

			setResourceState(renderTargets[i]);
		}

		if (depthStencil.texture)
		{
			setResourceState(depthStencil);

			depthStencilClearDesc.setHandle(depthStencil.dsvHandle);

			commandList->setRenderTargets(static_cast<uint32_t>(N), transientRTVHandles.data(), FALSE, &(depthStencil.dsvHandle));
		}
		else
		{
			commandList->setRenderTargets(static_cast<uint32_t>(N), transientRTVHandles.data(), FALSE, nullptr);
		}
	}

	template<size_t N>
	inline void GraphicsContext::setVertexBuffers(const uint32_t startSlot, const Resource::D3D12Resource::VertexBufferDesc(&vertexBuffers)[N])
	{
#ifdef _DEBUG
		if (startSlot + N > D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT)
		{
			LOGERROR(L"无法绑定更多的顶点缓冲");
		}
#endif // _DEBUG

		for (uint32_t i = 0; i < N; i++)
		{
			transientVBViews[i] = vertexBuffers[i].vbv;

			setResourceState(vertexBuffers[i]);
		}

		commandList->setVertexBuffers(startSlot, static_cast<uint32_t>(N), transientVBViews.data());
	}

	template<size_t N>
	inline void GraphicsContext::setShaderResources(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t targetSRVState)
	{
		for (const Resource::D3D12Resource::ShaderResourceDesc& desc : descs)
		{
			setResourceState(desc, targetSRVState);
		}
	}
}

#endif // !_GEAR_CORE_GRAPHICSCONTEXT_H_
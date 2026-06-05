#pragma once

#ifndef _GEAR_CORE_GRAPHICSCONTEXT_H_
#define _GEAR_CORE_GRAPHICSCONTEXT_H_

#include<Gear/Core/D3D12Core/CommandList.h>

#include<Gear/Core/D3D12Core/RootSignature.h>

#include<Gear/Core/D3D12Core/PipelineState.h>

#include<Gear/Core/Resource/BufferView.h>

#include<Gear/Core/Resource/StaticCBuffer.h>

#include<array>

#include<type_traits>

//创建一个作用域，内置默认值为0的临时偏移co，它会随着API调用自增，建议搭配CodeSnippets中的片段使用
//注意事项：作用域内只允许ctx->set*SConstants调用！因为宏定义是无法处理某些特殊情况的
#define SETCONSTS(...) { uint32_t co = 0u; __VA_ARGS__ }

namespace Gear::Core
{
	class GraphicsContext
	{
	public:

		GraphicsContext(const GraphicsContext&) = delete;

		void operator=(const GraphicsContext&) = delete;

		GraphicsContext();

		~GraphicsContext();

		void updateBuffer(Resource::BufferView& bufferView, const void* const data, const uint32_t size) const;

		void updateBuffer(Resource::StaticCBuffer& staticCBuffer, const void* const data, const uint32_t size) const;

		void setGlobalConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

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

		template<typename T>
		void setVSConstants(const T& t, uint32_t& offset) const;

		template<typename T>
		void setHSConstants(const T& t, uint32_t& offset) const;

		template<typename T>
		void setDSConstants(const T& t, uint32_t& offset) const;

		template<typename T>
		void setGSConstants(const T& t, uint32_t& offset) const;

		template<typename T>
		void setPSConstants(const T& t, uint32_t& offset) const;

		template<typename T>
		void setCSConstants(const T& t, uint32_t& offset) const;

		void constantsWriteCheck(const D3D12Core::RootSignature::ShaderType shaderType, const uint32_t numWrite) const;

		void setVSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setHSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setDSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setGSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setPSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setCSConstants(const uint32_t numValues, const void* const data, uint32_t& offset) const;

		void setVSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setHSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setDSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setGSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setPSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setCSConstantBuffer(const Resource::ImmutableCBuffer& immutableCBuffer);

		void setPipelineState(const D3D12Core::PipelineState& pipelineState);

		template<size_t N>
		void setRenderTargets(const Resource::D3D12Resource::RenderTargetDesc(&renderTargets)[N], const Resource::D3D12Resource::DepthStencilDesc& depthStencil = {});

		void setRenderTargets(const Resource::D3D12Resource::DepthStencilDesc& depthStencil);

		void setDefRenderTarget() const;

		void clearDefRenderTarget(const float clearValue[4]) const;

		//推迟到draw call进行清理
		void clearRenderTarget(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4]);

		//推迟到draw call进行清理
		void clearDepthStencil(const Resource::D3D12Resource::DepthStencilDesc& desc, const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil);

		//立刻进行清理
		void clearRenderTargetInstant(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4]);

		//立刻进行清理
		void clearDepthStencilInstant(const Resource::D3D12Resource::DepthStencilDesc& desc, const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil);

		template<size_t N>
		void setVertexBuffers(const uint32_t startSlot, const Resource::D3D12Resource::VertexBufferDesc(&vertexBuffers)[N]);

		void setIndexBuffer(const Resource::D3D12Resource::IndexBufferDesc& indexBuffers);

		void setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology);

		void setViewport(const float width, const float height);

		void setViewport(const uint32_t width, const uint32_t height);

		void setScissorRect(const uint32_t left, const uint32_t top, const uint32_t right, const uint32_t bottom);

		void setScissorRect(const float left, const float top, const float right, const float bottom);

		void setViewportSimple(const float width, const float height);

		void setViewportSimple(const uint32_t width, const uint32_t height);

		void clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const float values[4]);

		void clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const uint32_t values[4]);

		void draw(const uint32_t vertexCountPerInstance, const uint32_t instanceCount, const uint32_t startVertexLocation, const uint32_t startInstanceLocation);

		void drawIndexed(const uint32_t indexCountPerInstance, const uint32_t instanceCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation, const uint32_t startInstanceLocation);

		void dispatch(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ);

		template<size_t N>
		void uavBarrier(const Resource::D3D12Resource::D3D12ResourceBase* const(&resources)[N]);

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
			const uint32_t rootParameterIndex;

			const D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
		};

		struct RenderTargetClearDesc
		{
			const D3D12_CPU_DESCRIPTOR_HANDLE handle;

			const float clearValue[4];
		};

		struct DepthStencilClearDesc
		{
			const D3D12_CPU_DESCRIPTOR_HANDLE handle;

			const D3D12_CLEAR_FLAGS flags;

			const float depth;

			const uint8_t stencil;
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

		void flushDepthStencilClearDescs();

		void setGraphicsRootSignature(const D3D12Core::RootSignature* const rootSignature);

		void setComputeRootSignature(const D3D12Core::RootSignature* const rootSignature);

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

		UniquePtr<D3D12Core::CommandList> commandList;

		std::vector<RootConstantBufferDesc> rootConstantBufferDescs;

		std::vector<RenderTargetClearDesc> renderTargetClearDescs;

		std::vector<DepthStencilClearDesc> depthStencilClearDescs;

		//这些是内部追踪的状态，用于减少图形API的调用
		const D3D12Core::PipelineState* currentPipelineState;

		const Resource::ImmutableCBuffer* userDefinedGlobalConstantBuffer;

		D3D_PRIMITIVE_TOPOLOGY primitiveTopology;

		const D3D12Core::RootSignature* graphicsRootSignature;

		const D3D12Core::RootSignature* computeRootSignature;

		std::array<uint32_t, 32> transientResourceIndices;

		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> transientRTVHandles;

		std::array<D3D12_VERTEX_BUFFER_VIEW, D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> transientVBViews;

		std::array<D3D12_RESOURCE_BARRIER, 32> transientUAVBarriers;

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

	template<typename T>
	inline void GraphicsContext::setVSConstants(const T& t, uint32_t& offset) const
	{
		static_assert(std::is_class<T>::value, "T must be a struct/class type");

		const uint32_t numElements = sizeof(T) / sizeof(uint32_t);

		setVSConstants(numElements, &t, offset);
	}

	template<typename T>
	inline void GraphicsContext::setHSConstants(const T& t, uint32_t& offset) const
	{
		static_assert(std::is_class<T>::value, "T must be a struct/class type");

		const uint32_t numElements = sizeof(T) / sizeof(uint32_t);

		setHSConstants(numElements, &t, offset);
	}

	template<typename T>
	inline void GraphicsContext::setDSConstants(const T& t, uint32_t& offset) const
	{
		static_assert(std::is_class<T>::value, "T must be a struct/class type");

		const uint32_t numElements = sizeof(T) / sizeof(uint32_t);

		setDSConstants(numElements, &t, offset);
	}

	template<typename T>
	inline void GraphicsContext::setGSConstants(const T& t, uint32_t& offset) const
	{
		static_assert(std::is_class<T>::value, "T must be a struct/class type");

		const uint32_t numElements = sizeof(T) / sizeof(uint32_t);

		setGSConstants(numElements, &t, offset);
	}

	template<typename T>
	inline void GraphicsContext::setPSConstants(const T& t, uint32_t& offset) const
	{
		static_assert(std::is_class<T>::value, "T must be a struct/class type");

		const uint32_t numElements = sizeof(T) / sizeof(uint32_t);

		setPSConstants(numElements, &t, offset);
	}

	template<typename T>
	inline void GraphicsContext::setCSConstants(const T& t, uint32_t& offset) const
	{
		static_assert(std::is_class<T>::value, "T must be a struct/class type");

		const uint32_t numElements = sizeof(T) / sizeof(uint32_t);

		setCSConstants(numElements, &t, offset);
	}

	template<size_t N>
	inline void GraphicsContext::getResourceIndicesFromDescs(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N])
	{
		for (uint32_t i = 0; i < N; i++)
		{
			transientResourceIndices[i] = descs[i].resourceIndex;
		}
	}

	template<size_t N>
	inline void GraphicsContext::setRenderTargets(const Resource::D3D12Resource::RenderTargetDesc(&renderTargets)[N], const Resource::D3D12Resource::DepthStencilDesc& depthStencil)
	{
		for (uint32_t i = 0; i < N; i++)
		{
			transientRTVHandles[i] = renderTargets[i].rtvHandle;

			setResourceState(renderTargets[i]);
		}

		if (depthStencil.texture)
		{
			setResourceState(depthStencil);

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
		for (uint32_t i = 0; i < N; i++)
		{
			transientVBViews[i] = vertexBuffers[i].vbv;

			setResourceState(vertexBuffers[i]);
		}

		commandList->setVertexBuffers(startSlot, static_cast<uint32_t>(N), transientVBViews.data());
	}

	template<size_t N>
	inline void GraphicsContext::uavBarrier(const Resource::D3D12Resource::D3D12ResourceBase* const(&resources)[N])
	{
		for (uint32_t i = 0; i < N; i++)
		{
			transientUAVBarriers[i] = CD3DX12_RESOURCE_BARRIER::UAV(resources[i]->getResource());
		}

		commandList->resourceBarrier(static_cast<uint32_t>(N), transientUAVBarriers.data());
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
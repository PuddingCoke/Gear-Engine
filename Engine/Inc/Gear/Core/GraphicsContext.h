#pragma once

#ifndef _GEAR_CORE_GRAPHICSCONTEXT_H_
#define _GEAR_CORE_GRAPHICSCONTEXT_H_

#include<Gear/Core/D3D12Core/CommandList.h>

#include<Gear/Core/D3D12Core/RootSignature.h>

#include<Gear/Core/D3D12Core/PipelineState.h>

#include<Gear/Core/Resource/BufferView.h>

#include<Gear/Core/Resource/StaticCBuffer.h>

namespace Gear
{
	namespace Core
	{
		class GraphicsContext
		{
		public:

			GraphicsContext(const GraphicsContext&) = delete;

			void operator=(const GraphicsContext&) = delete;

			GraphicsContext();

			~GraphicsContext();

			void updateBuffer(Resource::BufferView* const bufferView, const void* const data, const uint32_t size) const;

			void updateBuffer(Resource::StaticCBuffer* const staticCBuffer, const void* const data, const uint32_t size) const;

			void setGlobalConstantBuffer(const Resource::ImmutableCBuffer* const immutableCBuffer);

			template<size_t N>
			void setVSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset);

			template<size_t N>
			void setHSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset);

			template<size_t N>
			void setDSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset);

			template<size_t N>
			void setGSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset);

			template<size_t N>
			void setPSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset);

			template<size_t N>
			void setCSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset);

			void setVSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const;

			void setHSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const;

			void setDSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const;

			void setGSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const;

			void setPSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const;

			void setCSConstants(const uint32_t numValues, const void* const data, const uint32_t offset) const;

			void setVSConstantBuffer(const Resource::ImmutableCBuffer* const immutableCBuffer);

			void setHSConstantBuffer(const Resource::ImmutableCBuffer* const immutableCBuffer);

			void setDSConstantBuffer(const Resource::ImmutableCBuffer* const immutableCBuffer);

			void setGSConstantBuffer(const Resource::ImmutableCBuffer* const immutableCBuffer);

			void setPSConstantBuffer(const Resource::ImmutableCBuffer* const immutableCBuffer);

			void setCSConstantBuffer(const Resource::ImmutableCBuffer* const immutableCBuffer);

			//绑定资源后必须调用这个方法！！
			void transitionResources();

			void setPipelineState(const D3D12Core::PipelineState* const pipelineState);

			void makeUserDefinedGlobalConstantBufferInvalid();

			//让内部追踪的管线状态失效
			//保证下次调用setPipelineState时必定调用图形API绑定管线状态
			void makePipelineStateInvalid();

			//同理
			void makeGraphicsRootSignatureInvalid();

			//同理
			void makeComputeRootSignatureInvalid();

			//重置内部追踪的状态
			void resetTrackedStates();

			template<size_t N>
			void setRenderTargets(const Resource::D3D12Resource::RenderTargetDesc(&renderTargets)[N], const Resource::D3D12Resource::DepthStencilDesc& depthStencil = {});

			void setRenderTargets(const Resource::D3D12Resource::DepthStencilDesc& depthStencil);

			void setDefRenderTarget() const;

			void clearDefRenderTarget(const float clearValue[4]) const;

			void clearRenderTarget(const Resource::D3D12Resource::RenderTargetDesc& desc, const float clearValue[4]) const;

			void clearDepthStencil(const Resource::D3D12Resource::DepthStencilDesc& desc, const D3D12_CLEAR_FLAGS flags, const float depth, const uint8_t stencil) const;

			template<size_t N>
			void setVertexBuffers(const uint32_t startSlot, const Resource::D3D12Resource::VertexBufferDesc(&vertexBuffers)[N]);

			void setIndexBuffer(const Resource::D3D12Resource::IndexBufferDesc& indexBuffers);

			void setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology) const;

			void setViewport(const float width, const float height);

			void setViewport(const uint32_t width, const uint32_t height);

			void setScissorRect(const uint32_t left, const uint32_t top, const uint32_t right, const uint32_t bottom);

			void setScissorRect(const float left, const float top, const float right, const float bottom);

			void setViewportSimple(const float width, const float height);

			void setViewportSimple(const uint32_t width, const uint32_t height);

			void clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const float values[4]);

			void clearUnorderedAccess(const Resource::D3D12Resource::ClearUAVDesc& desc, const uint32_t values[4]);

			void draw(const uint32_t vertexCountPerInstance, const uint32_t instanceCount, const uint32_t startVertexLocation, const uint32_t startInstanceLocation) const;

			void drawIndexed(const uint32_t indexCountPerInstance, const uint32_t instanceCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation, const uint32_t startInstanceLocation) const;

			void dispatch(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ) const;

			template<size_t N>
			void uavBarrier(const Resource::D3D12Resource::D3D12ResourceBase* const(&resources)[N]);

			void begin();

			D3D12Core::CommandList* getCommandList() const;

		private:

			struct RootConstantBufferDesc
			{
				enum Type
				{
					GRAPHICS, COMPUTE
				} type;

				uint32_t rootParameterIndex;

				D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
			};

			template<size_t N>
			void setShaderResources(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t targetSRVState);

			//从提供的ShaderResourceDesc提取索引
			template<size_t N>
			void getResourceIndicesFromDescs(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N]);

			void pushRootConstantBufferDesc(const RootConstantBufferDesc& desc);

			void setGraphicsRootSignature(const D3D12Core::RootSignature* const rootSignature);

			void setComputeRootSignature(const D3D12Core::RootSignature* const rootSignature);

			//根据Desc设置对应资源的转变状态

			void setResourceState(const Resource::D3D12Resource::ShaderResourceDesc& desc, const uint32_t targetSRVState);

			void setResourceState(const Resource::D3D12Resource::RenderTargetDesc& desc);

			void setResourceState(const Resource::D3D12Resource::DepthStencilDesc& desc);

			void setResourceState(const Resource::D3D12Resource::VertexBufferDesc& desc);

			void setResourceState(const Resource::D3D12Resource::IndexBufferDesc& desc);

			Resource::D3D12Resource::D3D12ResourceBase* setResourceState(const Resource::D3D12Resource::ClearUAVDesc& desc);

			D3D12_VIEWPORT vp;

			D3D12_RECT rt;

			D3D12Core::CommandList* const commandList;

			uint32_t resourceIndices[32];

			std::vector<RootConstantBufferDesc> rootConstantBufferDescs;

			const Resource::ImmutableCBuffer* userDefinedGlobalConstantBuffer;

			const D3D12Core::PipelineState* currentPipelineState;

			const D3D12Core::RootSignature* graphicsRootSignature;

			const D3D12Core::RootSignature* computeRootSignature;

			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> transientRTVHandles;

			std::vector<D3D12_VERTEX_BUFFER_VIEW> transientVBViews;

			std::vector<D3D12_RESOURCE_BARRIER> transientUAVBarriers;

		};

		template<size_t N>
		inline void GraphicsContext::setVSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset)
		{
			getResourceIndicesFromDescs(descs);

			setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			setVSConstants(N, resourceIndices, offset);
		}

		template<size_t N>
		inline void GraphicsContext::setHSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset)
		{
			getResourceIndicesFromDescs(descs);

			setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			setHSConstants(N, resourceIndices, offset);
		}

		template<size_t N>
		inline void GraphicsContext::setDSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset)
		{
			getResourceIndicesFromDescs(descs);

			setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			setDSConstants(N, resourceIndices, offset);
		}

		template<size_t N>
		inline void GraphicsContext::setGSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset)
		{
			getResourceIndicesFromDescs(descs);

			setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			setGSConstants(N, resourceIndices, offset);
		}

		template<size_t N>
		inline void GraphicsContext::setPSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset)
		{
			getResourceIndicesFromDescs(descs);

			setShaderResources(descs, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			setPSConstants(N, resourceIndices, offset);
		}

		template<size_t N>
		inline void GraphicsContext::setCSConstants(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N], const uint32_t offset)
		{
			getResourceIndicesFromDescs(descs);

			setShaderResources(descs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			setCSConstants(N, resourceIndices, offset);
		}

		template<size_t N>
		inline void GraphicsContext::getResourceIndicesFromDescs(const Resource::D3D12Resource::ShaderResourceDesc(&descs)[N])
		{
			for (uint32_t i = 0; i < N; i++)
			{
				resourceIndices[i] = descs[i].resourceIndex;
			}
		}

		template<size_t N>
		inline void GraphicsContext::setRenderTargets(const Resource::D3D12Resource::RenderTargetDesc(&renderTargets)[N], const Resource::D3D12Resource::DepthStencilDesc& depthStencil)
		{
			transientRTVHandles.clear();

			for (const Resource::D3D12Resource::RenderTargetDesc& desc : renderTargets)
			{
				transientRTVHandles.emplace_back(desc.rtvHandle);

				setResourceState(desc);
			}

			if (depthStencil.texture)
			{
				setResourceState(depthStencil);

				commandList->setRenderTargets(static_cast<uint32_t>(transientRTVHandles.size()), transientRTVHandles.data(), FALSE, &(depthStencil.dsvHandle));
			}
			else
			{
				commandList->setRenderTargets(static_cast<uint32_t>(transientRTVHandles.size()), transientRTVHandles.data(), FALSE, nullptr);
			}
		}

		template<size_t N>
		inline void GraphicsContext::setVertexBuffers(const uint32_t startSlot, const Resource::D3D12Resource::VertexBufferDesc(&vertexBuffers)[N])
		{
			transientVBViews.clear();

			for (const Resource::D3D12Resource::VertexBufferDesc& desc : vertexBuffers)
			{
				transientVBViews.emplace_back(desc.vbv);

				setResourceState(desc);
			}

			commandList->setVertexBuffers(startSlot, static_cast<uint32_t>(transientVBViews.size()), transientVBViews.data());
		}

		template<size_t N>
		inline void GraphicsContext::uavBarrier(const Resource::D3D12Resource::D3D12ResourceBase* const(&resources)[N])
		{
			transientUAVBarriers.clear();

			for (const Resource::D3D12Resource::D3D12ResourceBase* const resource : resources)
			{
				const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource->getResource());

				transientUAVBarriers.emplace_back(barrier);
			}

			commandList->resourceBarrier(static_cast<uint32_t>(transientUAVBarriers.size()), transientUAVBarriers.data());
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
}

#endif // !_GEAR_CORE_GRAPHICSCONTEXT_H_
#ifndef _GEAR_CORE_GRAPHICSCOMMANDEXECUTER_H_
#define _GEAR_CORE_GRAPHICSCOMMANDEXECUTER_H_

#include <Gear/Core/D3D12Core/GraphicsCommandList.h>

#include <Gear/Core/D3D12Core/RootSignature.h>

#include <Gear/Core/D3D12Core/GraphicsState.h>

#include <Gear/Core/D3D12Core/ComputeState.h>

#include <Gear/Resource/BufferView.h>

#include <Gear/Resource/DefaultCBuffer.h>

// #include <Gear/Resource/IndexTable.h>

#include <Gear/Utils/StaticVector.h>

#include <Gear/Core/FMT.h>

namespace Gear::Core
{
	CREATESAFETYPE(GraphicsCommandExecuter);

	class GraphicsCommandExecuter
	{
	public:

		GraphicsCommandExecuter(const GraphicsCommandExecuter&) = delete;

		void operator=(const GraphicsCommandExecuter&) = delete;

		GraphicsCommandExecuter();

		//将上传堆中的数据拷贝到缓冲
		void updateBuffer(D3D12Resource::Buffer* const buffer,
		                  D3D12Resource::UploadHeap* const uploadHeap,
		                  const uint64_t size) const;

		void setPipelineState(D3D12Core::GraphicsState& graphicsPipelineState);

		void setPipelineState(D3D12Core::ComputeState& computePipelineState);

		//以下的方法用于设置渲染目标

		void setRenderTargets(const uint64_t renderTargetDescsPtrOffset,
		                      const uint32_t numRenderTargets,
		                      const Resource::DepthStencilDesc& depthStencil);

		void setDefRenderTarget();

		//清理后备缓冲
		void clearDefRenderTarget(const float clearValue[4]) const;

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
		void setVertexBuffers(const uint64_t vertexBufferDescsPtrOffset,
		                      const uint32_t numVertexBuffers,
		                      const uint32_t startSlot);

		//设置索引缓冲
		void setIndexBuffer(const Resource::IndexBufferDesc& indexBuffer);

		//设置图元拓扑
		void setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology);

		//设置视口
		void setViewport(const float width, const float height);

		//设置剪刀矩形
		void setScissorRect(const int32_t left, const int32_t top, const int32_t right, const int32_t bottom);

		//以下的方法用于设置每个着色器独享的根常量
		//GraphicsContext(GraphicsCommandGenerator) 需要保证 setShaderConstants 调用仅在 Draw/Dispatch 之前

		template<D3D12Core::ShaderType ShaderType>
		void setShaderConstants(const uint64_t dataPtrOffset,
		                        const uint32_t numConstantValues,
		                        const uint32_t constantWriteOffset);

		template<D3D12Core::ShaderType ShaderType>
		void setConstantBuffer(const Resource::ImmutableCBuffer& constantBuffer);

		// template<D3D12Core::ShaderType ShaderType>
		// void setIndexTable(Resource::IndexTable& indexTable);

		void setAllShaderResources(const uint64_t allShaderResourceDescPtrOffset,
		                           const uint32_t numAllShaderResourceDesc,
		                           const uint32_t stateToOverride = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		void setPixelShaderResources(const uint64_t pixelShaderResourceDescPtrOffset,
		                             const uint32_t numPixelShaderResourceDesc);

		void setNonPixelShaderResources(const uint64_t nonPixelShaderResourceDescPtrOffset,
		                                const uint32_t numNonPixelShaderResourceDesc);

		void draw(const uint32_t vertexCountPerInstance,
		          const uint32_t instanceCount,
		          const uint32_t startVertexLocation,
		          const uint32_t startInstanceLocation);

		void drawIndexed(const uint32_t indexCountPerInstance,
		                 const uint32_t instanceCount,
		                 const uint32_t startIndexLocation,
		                 const int32_t baseVertexLocation,
		                 const uint32_t startInstanceLocation);

		void dispatchGrp(const uint32_t threadGroupCountX,
		                 const uint32_t threadGroupCountY,
		                 const uint32_t threadGroupCountZ);

		void dispatchDim(const uint32_t dispatchThreadCountX,
		                 const uint32_t dispatchThreadCountY,
		                 const uint32_t dispatchThreadCountZ);

		//清理UAV
		void clearUnorderedAccess(const Resource::UAVClearDesc& desc, const float values[4]);

		//清理UAV
		void clearUnorderedAccess(const Resource::UAVClearDesc& desc, const uint32_t values[4]);

		void begin();

		void resetTrackedStates();

		void resetPipelineState();

		void resetTrackedGraphicsStates();

		void resetTrackedComputeStates();

		D3D12Core::GraphicsCommandList* getCommandList() const;

		void pushVolatileResource(Resource::ResourceBase* const resource);

		void pushUnsettedIndex(const D3D12Core::ShaderType shaderType,
		                       const uint32_t* indexAddress,
		                       const uint64_t location);

		uint64_t getShaderConstantsWriteLocation(const D3D12Core::ShaderType shaderType) const;

		uint64_t getAllShaderResourceDescsWriteLocation() const;

		uint64_t getPixelShaderResourceDescsWriteLocation() const;

		uint64_t getNonPixelShaderResourceDescsWriteLocation() const;

		uint64_t getRenderTargetDescsWriteLocation() const;

		uint64_t getVertexBufferDescsWriteLocation() const;

		void pushShaderConstants(const D3D12Core::ShaderType shaderType,
		                         const uint32_t* const srcBegin,
		                         const uint32_t count);

		void pushAllShaderResourceDesc(const Resource::ShaderResourceDesc& desc);

		void pushPixelShaderResourceDesc(const Resource::ShaderResourceDesc& desc);

		void pushNonPixelShaderResourceDesc(const Resource::ShaderResourceDesc& desc);

		void pushRenderTargetDesc(const Resource::RenderTargetDesc& desc);

		void pushVertexBufferDesc(const Resource::VertexBufferDesc& desc);

		void prepare();

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

		class DepthStencilClearDesc
		{
		public:

			DepthStencilClearDesc() = default;

			~DepthStencilClearDesc() = default;

			void setHandle(const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) noexcept;

			void setClearData(const D3D12_CLEAR_FLAGS clearFlags,
			                  const float clearDepth,
			                  const uint8_t clearStencil) noexcept;

			void reset() noexcept;

			D3D12_CPU_DESCRIPTOR_HANDLE getHandle() const noexcept;

			D3D12_CLEAR_FLAGS getFlags() const noexcept;

			float getDepth() const noexcept;

			uint8_t getStencil() const noexcept;

		private:

			D3D12_CPU_DESCRIPTOR_HANDLE handle = {};

			uint32_t flags = 0;

			float depth = 0.f;

			uint8_t stencil = 0;
		};

		void setPipelineState(ID3D12PipelineState* const pipelineState);

		void setShaderResources(const Resource::ShaderResourceDesc* const descs,
		                        const uint32_t numDescs,
		                        const uint32_t targetSRVState);

		void pushRootConstantBufferDesc(const RootConstantBufferDesc& desc);

		template<bool IsGraphicsRootSignature>
		void flushRootConstantBufferDescs();

		void flushRenderTargetClearDescs();

		void resetDepthStencilClearDesc();

		void setGraphicsRootSignature(const D3D12Core::RootSignature* const rootSignature);

		void setComputeRootSignature(const D3D12Core::RootSignature* const rootSignature);

		const D3D12Core::RootSignature* getGraphicsRootSignature() const;

		const D3D12Core::RootSignature* getComputeRootSignature() const;

		void setResourceState(const Resource::ShaderResourceDesc& desc, const uint32_t targetSRVState);

		void setResourceState(const Resource::RenderTargetDesc& desc);

		void setResourceState(const Resource::DepthStencilDesc& desc);

		void setResourceState(const Resource::VertexBufferDesc& desc);

		void setResourceState(const Resource::IndexBufferDesc& desc);

		D3D12Resource::D3D12ResourceBase* setResourceState(const Resource::UAVClearDesc& desc);

		void prepareDrawCallResources();

		void prepareDispatchCallResources();

		void resetGraphicsRootSignature();

		void resetPrimitiveTopology();

		void resetComputeRootSignature();

		void constantsWriteCheck(const D3D12Core::ShaderType shaderType,
		                         const uint32_t numWrite,
		                         const uint32_t offset) const;

		// void prepareIndexTables();

		D3D12_VIEWPORT vp;

		D3D12_RECT rt;

		D3D12Core::GraphicsCommandListPtr commandList;

		//以下是内部追踪的状态，用于减少图形API的调用

		ID3D12PipelineState* currentPipelineState;

		D3D_PRIMITIVE_TOPOLOGY primitiveTopology;

		const D3D12Core::RootSignature* graphicsRootSignature;

		const D3D12Core::RootSignature* computeRootSignature;

		//以下是用于每次draw call或dispatch call的临时资源

		//这里有个假设，微软的官方文档说每个根签名最多有 64 个 DWORD
		Utils::StaticVector<RootConstantBufferDesc,
		                    D3D12Core::RootSignature::maxDWORD / D3D12Core::RootSignature::perDescriptorDWORD>
			rootConstantBufferDescs;

		Utils::StaticVector<RenderTargetClearDesc, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT>
			renderTargetClearDescs;

		DepthStencilClearDesc depthStencilClearDesc;

		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> transientRTVHandles;

		std::array<DXGI_FORMAT, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> transientRTVFormats;

		uint32_t transientNumRTV;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE transientPrimitiveTopologyType;

		DXGI_FORMAT transientDSVFormat;

		bool switchGraphicsPipelineState;

		std::array<D3D12_VERTEX_BUFFER_VIEW, D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> transientVBViews;

		union
		{
			D3D12Core::GraphicsState* graphicsState;

			D3D12Core::ComputeState* computeState;
		};

		std::vector<Resource::ResourceBase*> volatileResourceList;

		struct UnsettedIndex
		{
			//用于取回索引的地址
			const uint32_t* indexAddress;

			//在 shaderConstants 中的位置
			uint64_t location;
		};

		//存储暂时未确定的索引
		std::vector<UnsettedIndex> unsettedIndices[static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT)];

		//缓存一帧需要的所有着色器常量
		std::vector<uint32_t> shaderConstants[static_cast<uint32_t>(D3D12Core::ShaderType::TYPECOUNT)];

		//按 Draw/Dispatch Call 顺序缓存一帧需要状态转变的像素同时又是非像素着色器资源
		std::vector<Resource::ShaderResourceDesc> allShaderResourceDescs;

		//按 Draw/Dispatch Call 顺序缓存一帧需要状态转变的像素着色器资源
		std::vector<Resource::ShaderResourceDesc> pixelShaderResourceDescs;

		//按 Draw/Dispatch Call 顺序缓存一帧需要状态转变的非像素着色器资源
		std::vector<Resource::ShaderResourceDesc> nonPixelShaderResourceDescs;

		//按 SetRenderTargets 调用顺序缓存一帧需要设置的渲染目标
		std::vector<Resource::RenderTargetDesc> renderTargetDescs;

		//按 SetVertexBuffers 调用顺序缓存一帧需要设置的顶点缓冲
		std::vector<Resource::VertexBufferDesc> vertexBufferDescs;

		// //存储检查过的是否需要更新的索引表来防止多次检查
		// std::vector<Resource::IndexTable*> indexTableCheckedList;

		// //存储需要更新的索引表
		// std::vector<Resource::IndexTable*> indexTableUpdateList;
	};

	template<D3D12Core::ShaderType ShaderType>
	void GraphicsCommandExecuter::setShaderConstants(const uint64_t dataPtrOffset,
	                                                 const uint32_t numConstantValues,
	                                                 const uint32_t constantWriteOffset)
	{
#ifdef _DEBUG
		constantsWriteCheck(ShaderType, numConstantValues, constantWriteOffset);
#endif

		const uint32_t shaderIndex = static_cast<uint32_t>(ShaderType);

		uint32_t rootParameterIndex = 0;

		if constexpr (ShaderType == D3D12Core::ShaderType::VERTEX)
		{
			rootParameterIndex = getGraphicsRootSignature()->getVSConstantsParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::HULL)
		{
			rootParameterIndex = getGraphicsRootSignature()->getHSConstantsParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::DOMAIN)
		{
			rootParameterIndex = getGraphicsRootSignature()->getDSConstantsParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::GEOMETRY)
		{
			rootParameterIndex = getGraphicsRootSignature()->getGSConstantsParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::PIXEL)
		{
			rootParameterIndex = getGraphicsRootSignature()->getPSConstantsParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::COMPUTE)
		{
			rootParameterIndex = getComputeRootSignature()->getCSConstantsParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::AMPLIFICATION)
		{
			rootParameterIndex = getGraphicsRootSignature()->getASConstantsParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::MESH)
		{
			rootParameterIndex = getGraphicsRootSignature()->getMSConstantsParameterIndex();
		}

		if constexpr (ShaderType == D3D12Core::ShaderType::COMPUTE)
		{
			commandList->setComputeRootConstants(rootParameterIndex,
			                                     numConstantValues,
			                                     shaderConstants[shaderIndex].data() + dataPtrOffset,
			                                     constantWriteOffset);
		}
		else
		{
			commandList->setGraphicsRootConstants(rootParameterIndex,
			                                      numConstantValues,
			                                      shaderConstants[shaderIndex].data() + dataPtrOffset,
			                                      constantWriteOffset);
		}
	}

	template<D3D12Core::ShaderType ShaderType>
	void GraphicsCommandExecuter::setConstantBuffer(const Resource::ImmutableCBuffer& constantBuffer)
	{
		D3D12Resource::Buffer* const buffer = constantBuffer.getBuffer();

		commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		uint32_t rootParameterIndex = 0;

		if constexpr (ShaderType == D3D12Core::ShaderType::ALL)
		{
			rootParameterIndex = D3D12Core::RootSignature::getUserGlobalConstantBufferParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::VERTEX)
		{
			rootParameterIndex = getGraphicsRootSignature()->getVSConstantBufferParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::HULL)
		{
			rootParameterIndex = getGraphicsRootSignature()->getHSConstantBufferParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::DOMAIN)
		{
			rootParameterIndex = getGraphicsRootSignature()->getDSConstantBufferParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::GEOMETRY)
		{
			rootParameterIndex = getGraphicsRootSignature()->getGSConstantBufferParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::PIXEL)
		{
			rootParameterIndex = getGraphicsRootSignature()->getPSConstantBufferParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::COMPUTE)
		{
			rootParameterIndex = getComputeRootSignature()->getCSConstantBufferParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::AMPLIFICATION)
		{
			rootParameterIndex = getGraphicsRootSignature()->getASConstantBufferParameterIndex();
		}
		else if constexpr (ShaderType == D3D12Core::ShaderType::MESH)
		{
			rootParameterIndex = getGraphicsRootSignature()->getMSConstantBufferParameterIndex();
		}

		pushRootConstantBufferDesc({ rootParameterIndex, constantBuffer.getGPUAddress() });
	}

	// template<D3D12Core::ShaderType ShaderType>
	// void GraphicsCommandExecuter::setIndexTable(Resource::IndexTable& indexTable)
	// {
	// 	D3D12Resource::Buffer* const buffer = indexTable.getBuffer();

	// 	//用户指定需要更新
	// 	if (indexTable.getUserUpdate())
	// 	{
	// 		if (buffer != nullptr)
	// 		{
	// 			if (!indexTable.getInCheckedList() && !indexTable.getInUpdateList())
	// 			{
	// 				const bool updated = indexTable.updateIndices();

	// 				indexTableCheckedList.push_back(&indexTable);

	// 				indexTable.pushToCheckedList();

	// 				//如果真的更新了索引，那么表示缓冲区需要被更新
	// 				if (updated)
	// 				{
	// 					commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_COPY_DEST);

	// 					indexTableUpdateList.push_back(&indexTable);

	// 					indexTable.pushToUpdateList();
	// 				}
	// 			}
	// 		}
	// 		else
	// 		{
	// 			if (!indexTable.getInCheckedList())
	// 			{
	// 				//对于动态类型的索引常量缓冲来说，每帧都需要获取可用位置并更新数据指针指向的内容就行
	// 				indexTable.updateIndices();

	// 				indexTableCheckedList.push_back(&indexTable);

	// 				indexTable.pushToCheckedList();
	// 			}
	// 		}
	// 	}
	// 	//用户未指定需要更新
	// 	else
	// 	{
	// 		if (buffer != nullptr)
	// 		{
	// 			//如果包含非持久性索引，那么需要进行隐式更新
	// 			if (indexTable.hasNonPersistentResource())
	// 			{
	// 				if (!indexTable.getInCheckedList() && !indexTable.getInUpdateList())
	// 				{
	// 					const bool updated = indexTable.updateIndices();

	// 					indexTableCheckedList.push_back(&indexTable);

	// 					indexTable.pushToCheckedList();

	// 					//如果真的更新了索引，那么表示缓冲区需要被更新
	// 					if (updated)
	// 					{
	// 						commandList->trackAndSetResourceState(buffer, D3D12_RESOURCE_STATE_COPY_DEST);

	// 						indexTableUpdateList.push_back(&indexTable);

	// 						indexTable.pushToUpdateList();
	// 					}
	// 				}
	// 			}
	// 			else
	// 			{
	// 				commandList->trackAndSetResourceState(buffer,
	// 				                                      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	// 			}
	// 		}
	// 		else
	// 		{
	// 			if (!indexTable.getInCheckedList())
	// 			{
	// 				//对于动态类型的索引常量缓冲来说，每帧都需要获取可用位置并更新数据指针指向的内容就行
	// 				indexTable.updateIndices();

	// 				indexTableCheckedList.push_back(&indexTable);

	// 				indexTable.pushToCheckedList();
	// 			}
	// 		}
	// 	}

	// 	uint32_t rootParameterIndex = 0;

	// 	if constexpr (ShaderType == D3D12Core::ShaderType::ALL)
	// 	{
	// 		rootParameterIndex = D3D12Core::RootSignature::getUserGlobalConstantBufferParameterIndex();
	// 	}
	// 	else if constexpr (ShaderType == D3D12Core::ShaderType::VERTEX)
	// 	{
	// 		rootParameterIndex = getGraphicsRootSignature()->getVSConstantBufferParameterIndex();
	// 	}
	// 	else if constexpr (ShaderType == D3D12Core::ShaderType::HULL)
	// 	{
	// 		rootParameterIndex = getGraphicsRootSignature()->getHSConstantBufferParameterIndex();
	// 	}
	// 	else if constexpr (ShaderType == D3D12Core::ShaderType::DOMAIN)
	// 	{
	// 		rootParameterIndex = getGraphicsRootSignature()->getDSConstantBufferParameterIndex();
	// 	}
	// 	else if constexpr (ShaderType == D3D12Core::ShaderType::GEOMETRY)
	// 	{
	// 		rootParameterIndex = getGraphicsRootSignature()->getGSConstantBufferParameterIndex();
	// 	}
	// 	else if constexpr (ShaderType == D3D12Core::ShaderType::PIXEL)
	// 	{
	// 		rootParameterIndex = getGraphicsRootSignature()->getPSConstantBufferParameterIndex();
	// 	}
	// 	else if constexpr (ShaderType == D3D12Core::ShaderType::COMPUTE)
	// 	{
	// 		rootParameterIndex = getComputeRootSignature()->getCSConstantBufferParameterIndex();
	// 	}
	// 	else if constexpr (ShaderType == D3D12Core::ShaderType::AMPLIFICATION)
	// 	{
	// 		rootParameterIndex = getGraphicsRootSignature()->getASConstantBufferParameterIndex();
	// 	}
	// 	else if constexpr (ShaderType == D3D12Core::ShaderType::MESH)
	// 	{
	// 		rootParameterIndex = getGraphicsRootSignature()->getMSConstantBufferParameterIndex();
	// 	}

	// 	pushRootConstantBufferDesc({ rootParameterIndex, indexTable.getGPUAddress() });
	// }

	template<bool IsGraphicsRootSignature>
	void GraphicsCommandExecuter::flushRootConstantBufferDescs()
	{
		if (rootConstantBufferDescs.size())
		{
			if constexpr (IsGraphicsRootSignature)
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

}

#endif // _GEAR_CORE_GRAPHICSCOMMANDEXECUTER_H_

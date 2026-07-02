#include<Gear/Core/D3D12Core/GraphicsCommandList.h>

#include<Gear/Core/Graphics.h>

namespace Gear::Core::D3D12Core
{
	GraphicsCommandList::GraphicsCommandList(const D3D12_COMMAND_LIST_TYPE commandListType) :
		CommandList(commandListType), currentDescriptorHeaps{ nullptr,nullptr }
	{
		if (commandListType != D3D12_COMMAND_LIST_TYPE_BUNDLE &&
			commandListType != D3D12_COMMAND_LIST_TYPE_COMPUTE &&
			commandListType != D3D12_COMMAND_LIST_TYPE_COPY &&
			commandListType != D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			LOGERROR("对于图形命令列表来说命令列表类型只能为",
				TOSTRING(D3D12_COMMAND_LIST_TYPE_BUNDLE),
				TOSTRING(D3D12_COMMAND_LIST_TYPE_COMPUTE),
				TOSTRING(D3D12_COMMAND_LIST_TYPE_COPY),
				TOSTRING(D3D12_COMMAND_LIST_TYPE_DIRECT));
		}

		CHECKERROR(GraphicsDevice::get()->CreateCommandList(0, commandListType, getCommandAllocator(), nullptr, IID_PPV_ARGS(&commandList)));

		setAndCloseCommandList(commandList.Get());
	}

	void GraphicsCommandList::resourceBarrier(const uint32_t numBarriers, const D3D12_RESOURCE_BARRIER* const pBarriers) const
	{
		commandList->ResourceBarrier(numBarriers, pBarriers);
	}

	void GraphicsCommandList::open()
	{
		currentDescriptorHeaps[0] = nullptr;

		currentDescriptorHeaps[1] = nullptr;

		resetCommandAllocator();

		commandList->Reset(getCommandAllocator(), nullptr);
	}

	void GraphicsCommandList::close()
	{
		commandList->Close();
	}

	ID3D12GraphicsCommandList6* GraphicsCommandList::get() const
	{
		return commandList.Get();
	}

	void GraphicsCommandList::setDescriptorHeap(ID3D12DescriptorHeap* const resourceHeap, ID3D12DescriptorHeap* const samplerHeap)
	{
		//SetDescriptorHeaps开销高，因此这里一定要去重
		if (resourceHeap != currentDescriptorHeaps[0] || samplerHeap != currentDescriptorHeaps[1])
		{
			currentDescriptorHeaps[0] = resourceHeap;

			currentDescriptorHeaps[1] = samplerHeap;

			commandList->SetDescriptorHeaps(_countof(currentDescriptorHeaps), currentDescriptorHeaps);
		}
	}

	void GraphicsCommandList::setPipelineState(ID3D12PipelineState* const pipelineState) const
	{
		commandList->SetPipelineState(pipelineState);
	}

	void GraphicsCommandList::setGraphicsRootSignature(ID3D12RootSignature* const rootSignature) const
	{
		commandList->SetGraphicsRootSignature(rootSignature);
	}

	void GraphicsCommandList::setComputeRootSignature(ID3D12RootSignature* const rootSignature) const
	{
		commandList->SetComputeRootSignature(rootSignature);
	}

	void GraphicsCommandList::setGraphicsRootConstantBuffer(const uint32_t rootParameterIndex, const D3D12_GPU_VIRTUAL_ADDRESS bufferLocation) const
	{
		commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, bufferLocation);
	}

	void GraphicsCommandList::setComputeRootConstantBuffer(const uint32_t rootParameterIndex, const D3D12_GPU_VIRTUAL_ADDRESS bufferLocation) const
	{
		commandList->SetComputeRootConstantBufferView(rootParameterIndex, bufferLocation);
	}

	void GraphicsCommandList::setGraphicsRootConstants(const uint32_t rootParameterIndex, const uint32_t num32BitValuesToSet, const void* const pSrcData, const uint32_t destOffsetIn32BitValues) const
	{
		commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, destOffsetIn32BitValues);
	}

	void GraphicsCommandList::setComputeRootConstants(const uint32_t rootParameterIndex, const uint32_t num32BitValuesToSet, const void* const pSrcData, const uint32_t destOffsetIn32BitValues) const
	{
		commandList->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, destOffsetIn32BitValues);
	}

	void GraphicsCommandList::drawInstanced(const uint32_t vertexCountPerInstance, const uint32_t instanceCount, const uint32_t startVertexLocation, const uint32_t startInstanceLocation) const
	{
		commandList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	}

	void GraphicsCommandList::drawIndexedInstanced(const uint32_t indexCountPerInstance, const uint32_t instanceCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation, const uint32_t startInstanceLocation) const
	{
		commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	void GraphicsCommandList::dispatch(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ) const
	{
		commandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	void GraphicsCommandList::setRenderTargets(const uint32_t numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* const pRenderTargetDescriptors, bool singleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* const pDepthStencilDescriptor) const
	{
		commandList->OMSetRenderTargets(numRenderTargetDescriptors, pRenderTargetDescriptors, singleHandleToDescriptorRange, pDepthStencilDescriptor);
	}

	void GraphicsCommandList::setDefRenderTarget() const
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = Graphics::getBackBufferHandle();

		setRenderTargets(1, &backBufferHandle, FALSE, nullptr);
	}

	void GraphicsCommandList::clearRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float colorRGBA[4], const uint32_t numRects, const D3D12_RECT* pRects) const
	{
		commandList->ClearRenderTargetView(renderTargetView, colorRGBA, numRects, pRects);
	}

	void GraphicsCommandList::clearDepthStencil(const D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, const D3D12_CLEAR_FLAGS clearFlags, const float depth, const uint8_t stencil, const uint32_t numRects, const D3D12_RECT* const pRects) const
	{
		commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, numRects, pRects);
	}

	void GraphicsCommandList::setVertexBuffers(const uint32_t startSlot, const uint32_t numViews, const D3D12_VERTEX_BUFFER_VIEW* const pViews) const
	{
		commandList->IASetVertexBuffers(startSlot, numViews, pViews);
	}

	void GraphicsCommandList::setIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* const pView) const
	{
		commandList->IASetIndexBuffer(pView);
	}

	void GraphicsCommandList::setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY primitiveTopology) const
	{
		commandList->IASetPrimitiveTopology(primitiveTopology);
	}

	void GraphicsCommandList::setViewports(const uint32_t numViewports, const D3D12_VIEWPORT* const pViewports) const
	{
		commandList->RSSetViewports(numViewports, pViewports);
	}

	void GraphicsCommandList::setScissorRects(const uint32_t numRects, const D3D12_RECT* const pRects) const
	{
		commandList->RSSetScissorRects(numRects, pRects);
	}

	void GraphicsCommandList::clearUnorderedAccessFloat(const D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, const D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, ID3D12Resource* const pResource, const float values[4], const uint32_t numRects, const D3D12_RECT* const pRects) const
	{
		commandList->ClearUnorderedAccessViewFloat(viewGPUHandleInCurrentHeap, viewCPUHandle, pResource, values, numRects, pRects);
	}

	void GraphicsCommandList::clearUnorderedAccessUint(const D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, const D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, ID3D12Resource* const pResource, const uint32_t values[4], const uint32_t numRects, const D3D12_RECT* const pRects) const
	{
		commandList->ClearUnorderedAccessViewUint(viewGPUHandleInCurrentHeap, viewCPUHandle, pResource, values, numRects, pRects);
	}

	void GraphicsCommandList::copyBufferRegion(D3D12Resource::Buffer* const dstBuffer, const uint64_t dstOffset, D3D12Resource::UploadHeap* srcBuffer, const uint64_t srcOffset, const uint64_t numBytes)
	{
		trackAndSetResourceState(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		flushResourceBarriers();

		commandList->CopyBufferRegion(dstBuffer->getResource(), dstOffset, srcBuffer->getResource(), srcOffset, numBytes);
	}

	void GraphicsCommandList::copyBufferRegion(D3D12Resource::Buffer* const dstBuffer, const uint64_t dstOffset, D3D12Resource::Buffer* srcBuffer, const uint64_t srcOffset, const uint64_t numBytes)
	{
		trackAndSetResourceState(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		trackAndSetResourceState(srcBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		flushResourceBarriers();

		commandList->CopyBufferRegion(dstBuffer->getResource(), dstOffset, srcBuffer->getResource(), srcOffset, numBytes);
	}

	void GraphicsCommandList::copyResource(D3D12Resource::Buffer* const dstBuffer, D3D12Resource::UploadHeap* const srcBuffer)
	{
		trackAndSetResourceState(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		flushResourceBarriers();

		commandList->CopyResource(dstBuffer->getResource(), srcBuffer->getResource());
	}

	void GraphicsCommandList::copyResource(D3D12Resource::Buffer* const dstBuffer, D3D12Resource::Buffer* const srcBuffer)
	{
		trackAndSetResourceState(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		trackAndSetResourceState(srcBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		flushResourceBarriers();

		commandList->CopyResource(dstBuffer->getResource(), srcBuffer->getResource());
	}

	void GraphicsCommandList::copyTextureRegion(D3D12Resource::Texture* const dstTexture, const uint32_t dstSubresource, D3D12Resource::Texture* const srcTexture, const uint32_t srcSubresource)
	{
		trackAndSetResourceState(dstTexture, D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COPY_DEST);

		trackAndSetResourceState(srcTexture, D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COPY_SOURCE);

		flushResourceBarriers();

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.pResource = dstTexture->getResource();
		dstLocation.SubresourceIndex = dstSubresource;

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.pResource = srcTexture->getResource();
		srcLocation.SubresourceIndex = srcSubresource;

		commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}
}

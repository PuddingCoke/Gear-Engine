#include<Gear/Core/D3D12Core/CommandList.h>

#include<Gear/Core/Graphics.h>

namespace Gear::Core::D3D12Core
{
	CommandList::CommandList(const D3D12_COMMAND_LIST_TYPE type)
	{
		allocators = makeUnique<ComPtr<ID3D12CommandAllocator>[]>(Graphics::getFrameBufferCount());

		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			GraphicsDevice::get()->CreateCommandAllocator(type, IID_PPV_ARGS(&allocators[i]));
		}

		GraphicsDevice::get()->CreateCommandList(0, type, allocators[Graphics::getFrameIndex()].Get(), nullptr, IID_PPV_ARGS(&commandList));

		commandList->Close();
	}

	CommandList::~CommandList()
	{
	}

	void CommandList::resourceBarrier(const uint32_t numBarriers, const D3D12_RESOURCE_BARRIER* const pBarriers) const
	{
		commandList->ResourceBarrier(numBarriers, pBarriers);
	}

	ID3D12GraphicsCommandList6* CommandList::get() const
	{
		return commandList.Get();
	}

	void CommandList::open() const
	{
		allocators[Graphics::getFrameIndex()].Get()->Reset();

		commandList->Reset(allocators[Graphics::getFrameIndex()].Get(), nullptr);
	}

	void CommandList::close() const
	{
		commandList->Close();
	}

	void CommandList::setDescriptorHeap(ID3D12DescriptorHeap* const resourceHeap, ID3D12DescriptorHeap* const samplerHeap) const
	{
		ID3D12DescriptorHeap* const descriptorHeaps[2] = { resourceHeap,samplerHeap };

		commandList->SetDescriptorHeaps(2, descriptorHeaps);
	}

	void CommandList::setPipelineState(ID3D12PipelineState* const pipelineState) const
	{
		commandList->SetPipelineState(pipelineState);
	}

	void CommandList::setGraphicsRootSignature(ID3D12RootSignature* const rootSignature) const
	{
		commandList->SetGraphicsRootSignature(rootSignature);
	}

	void CommandList::setComputeRootSignature(ID3D12RootSignature* const rootSignature) const
	{
		commandList->SetComputeRootSignature(rootSignature);
	}

	void CommandList::setGraphicsRootConstantBuffer(const uint32_t rootParameterIndex, const D3D12_GPU_VIRTUAL_ADDRESS bufferLocation) const
	{
		commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, bufferLocation);
	}

	void CommandList::setComputeRootConstantBuffer(const uint32_t rootParameterIndex, const D3D12_GPU_VIRTUAL_ADDRESS bufferLocation) const
	{
		commandList->SetComputeRootConstantBufferView(rootParameterIndex, bufferLocation);
	}

	void CommandList::setGraphicsRootConstants(const uint32_t rootParameterIndex, const uint32_t num32BitValuesToSet, const void* const pSrcData, const uint32_t destOffsetIn32BitValues) const
	{
		commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, destOffsetIn32BitValues);
	}

	void CommandList::setComputeRootConstants(const uint32_t rootParameterIndex, const uint32_t num32BitValuesToSet, const void* const pSrcData, const uint32_t destOffsetIn32BitValues) const
	{
		commandList->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, destOffsetIn32BitValues);
	}

	void CommandList::drawInstanced(const uint32_t vertexCountPerInstance, const uint32_t instanceCount, const uint32_t startVertexLocation, const uint32_t startInstanceLocation) const
	{
		commandList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	}

	void CommandList::drawIndexedInstanced(const uint32_t indexCountPerInstance, const uint32_t instanceCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation, const uint32_t startInstanceLocation) const
	{
		commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	}

	void CommandList::dispatch(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ) const
	{
		commandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	void CommandList::setRenderTargets(const uint32_t numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* const pRenderTargetDescriptors, bool singleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* const pDepthStencilDescriptor) const
	{
		commandList->OMSetRenderTargets(numRenderTargetDescriptors, pRenderTargetDescriptors, singleHandleToDescriptorRange, pDepthStencilDescriptor);
	}

	void CommandList::setDefRenderTarget() const
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = Graphics::getBackBufferHandle();

		setRenderTargets(1, &backBufferHandle, FALSE, nullptr);
	}

	void CommandList::clearRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float colorRGBA[4], const uint32_t numRects, const D3D12_RECT* pRects) const
	{
		commandList->ClearRenderTargetView(renderTargetView, colorRGBA, numRects, pRects);
	}

	void CommandList::clearDepthStencil(const D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, const D3D12_CLEAR_FLAGS clearFlags, const float depth, const uint8_t stencil, const uint32_t numRects, const D3D12_RECT* const pRects) const
	{
		commandList->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, numRects, pRects);
	}

	void CommandList::setVertexBuffers(const uint32_t startSlot, const uint32_t numViews, const D3D12_VERTEX_BUFFER_VIEW* const pViews) const
	{
		commandList->IASetVertexBuffers(startSlot, numViews, pViews);
	}

	void CommandList::setIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* const pView) const
	{
		commandList->IASetIndexBuffer(pView);
	}

	void CommandList::setPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY primitiveTopology) const
	{
		commandList->IASetPrimitiveTopology(primitiveTopology);
	}

	void CommandList::setViewports(const uint32_t numViewports, const D3D12_VIEWPORT* const pViewports) const
	{
		commandList->RSSetViewports(numViewports, pViewports);
	}

	void CommandList::setScissorRects(const uint32_t numRects, const D3D12_RECT* const pRects) const
	{
		commandList->RSSetScissorRects(numRects, pRects);
	}

	void CommandList::clearUnorderedAccessFloat(const D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, const D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, ID3D12Resource* const pResource, const float values[4], const uint32_t numRects, const D3D12_RECT* const pRects) const
	{
		commandList->ClearUnorderedAccessViewFloat(viewGPUHandleInCurrentHeap, viewCPUHandle, pResource, values, numRects, pRects);
	}

	void CommandList::clearUnorderedAccessUint(const D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, const D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, ID3D12Resource* const pResource, const uint32_t values[4], const uint32_t numRects, const D3D12_RECT* const pRects) const
	{
		commandList->ClearUnorderedAccessViewUint(viewGPUHandleInCurrentHeap, viewCPUHandle, pResource, values, numRects, pRects);
	}

	void CommandList::flushResourceBarriers()
	{
		ResourceStateTracker::flushResourceBarriers(commandList.Get());
	}

	void CommandList::copyBufferRegion(Resource::D3D12Resource::Buffer* const dstBuffer, const uint64_t dstOffset, Resource::D3D12Resource::UploadHeap* srcBuffer, const uint64_t srcOffset, const uint64_t numBytes)
	{
		trackAndSetResourceState(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		flushResourceBarriers();

		commandList->CopyBufferRegion(dstBuffer->getResource(), dstOffset, srcBuffer->getResource(), srcOffset, numBytes);
	}

	void CommandList::copyBufferRegion(Resource::D3D12Resource::Buffer* const dstBuffer, const uint64_t dstOffset, Resource::D3D12Resource::Buffer* srcBuffer, const uint64_t srcOffset, const uint64_t numBytes)
	{
		trackAndSetResourceState(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		trackAndSetResourceState(srcBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		flushResourceBarriers();

		commandList->CopyBufferRegion(dstBuffer->getResource(), dstOffset, srcBuffer->getResource(), srcOffset, numBytes);
	}

	void CommandList::copyResource(Resource::D3D12Resource::Buffer* const dstBuffer, Resource::D3D12Resource::UploadHeap* const srcBuffer)
	{
		trackAndSetResourceState(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		flushResourceBarriers();

		commandList->CopyResource(dstBuffer->getResource(), srcBuffer->getResource());
	}

	void CommandList::copyResource(Resource::D3D12Resource::Buffer* const dstBuffer, Resource::D3D12Resource::Buffer* const srcBuffer)
	{
		trackAndSetResourceState(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		trackAndSetResourceState(srcBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		flushResourceBarriers();

		commandList->CopyResource(dstBuffer->getResource(), srcBuffer->getResource());
	}

	void CommandList::copyTextureRegion(Resource::D3D12Resource::Texture* const dstTexture, const uint32_t dstSubresource, Resource::D3D12Resource::Texture* const srcTexture, const uint32_t srcSubresource)
	{
		trackAndSetResourceState(dstTexture, Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COPY_DEST);

		trackAndSetResourceState(srcTexture, Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COPY_SOURCE);

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


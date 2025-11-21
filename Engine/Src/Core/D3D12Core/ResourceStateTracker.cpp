#include<Gear/Core/D3D12Core/ResourceStateTracker.h>

void Gear::Core::D3D12Core::ResourceStateTracker::trackAndSetResourceState(Resource::D3D12Resource::Texture* const texture, const uint32_t mipslice, const uint32_t state)
{
	if (texture->getStateTracking())
	{
		pushResourceToTrackList(texture);

		if (mipslice == Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS)
		{
			texture->setAllState(state);
		}
		else
		{
			texture->setMipSliceState(mipslice, state);
		}
	}
}

void Gear::Core::D3D12Core::ResourceStateTracker::trackAndSetResourceState(Resource::D3D12Resource::Buffer* const buffer, const uint32_t state)
{
	if (buffer && buffer->getStateTracking())
	{
		pushResourceToTrackList(buffer);

		buffer->setState(state);
	}
}

void Gear::Core::D3D12Core::ResourceStateTracker::solvePendingBarriers(std::vector<D3D12_RESOURCE_BARRIER>& outBarriers)
{
	if (pendingBufferBarrier.size())
	{
		for (const Resource::D3D12Resource::PendingBufferBarrier& pendingBarrier : pendingBufferBarrier)
		{
			pendingBarrier.buffer->solvePendingBarrier(outBarriers, pendingBarrier.afterState);
		}

		pendingBufferBarrier.clear();
	}

	if (pendingTextureBarrier.size())
	{
		for (const Resource::D3D12Resource::PendingTextureBarrier& pendingBarrier : pendingTextureBarrier)
		{
			pendingBarrier.texture->solvePendingBarrier(outBarriers, pendingBarrier.mipSlice, pendingBarrier.afterState);
		}

		pendingTextureBarrier.clear();
	}
}

void Gear::Core::D3D12Core::ResourceStateTracker::updateReferredSharedResourceStates()
{
	if (referredResources.size())
	{
		for (Resource::D3D12Resource::D3D12ResourceBase* const res : referredResources)
		{
			res->updateGlobalStates();

			res->resetInternalStates();

			res->popFromReferredList();
		}

		referredResources.clear();
	}
}

void Gear::Core::D3D12Core::ResourceStateTracker::transitionResources(ID3D12GraphicsCommandList6* const commandList)
{
	if (transitionBuffers.size())
	{
		for (Resource::D3D12Resource::Buffer* const buff : transitionBuffers)
		{
			buff->transition(transitionBarriers, pendingBufferBarrier);

			buff->popFromTrackingList();
		}

		transitionBuffers.clear();
	}

	if (transitionTextures.size())
	{
		for (Resource::D3D12Resource::Texture* const tex : transitionTextures)
		{
			tex->transition(transitionBarriers, pendingTextureBarrier);

			tex->popFromTrackingList();
		}

		transitionTextures.clear();
	}

	if (transitionBarriers.size())
	{
		commandList->ResourceBarrier(static_cast<uint32_t>(transitionBarriers.size()), transitionBarriers.data());

		transitionBarriers.clear();
	}
}

void Gear::Core::D3D12Core::ResourceStateTracker::pushResourceToTrackList(Resource::D3D12Resource::Texture* const texture)
{
	texture->pushToReferredList(referredResources);

	texture->pushToTrackingList(transitionTextures);
}

void Gear::Core::D3D12Core::ResourceStateTracker::pushResourceToTrackList(Resource::D3D12Resource::Buffer* const buffer)
{
	buffer->pushToReferredList(referredResources);

	buffer->pushToTrackingList(transitionBuffers);
}
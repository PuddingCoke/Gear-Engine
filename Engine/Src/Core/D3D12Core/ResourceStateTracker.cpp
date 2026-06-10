#include<Gear/Core/D3D12Core/ResourceStateTracker.h>

namespace Gear::Core::D3D12Core
{
	void ResourceStateTracker::trackAndSetResourceState(Resource::D3D12Resource::Texture* const texture, const uint32_t mipslice, const uint32_t state)
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

	void ResourceStateTracker::trackAndSetResourceState(Resource::D3D12Resource::Buffer* const buffer, const uint32_t state)
	{
		if (buffer && buffer->getStateTracking())
		{
			pushResourceToTrackList(buffer);

			buffer->setState(state);
		}
	}

	void ResourceStateTracker::resolvePendingResourceStates(std::vector<D3D12_RESOURCE_BARRIER>& outBarriers)
	{
		if (pendingResources.size())
		{
			for (Resource::D3D12Resource::D3D12ResourceBase* const resource : pendingResources)
			{
				resource->resolvePendingState(outBarriers);

				resource->resetPendingState();

				resource->popFromPendingList();
			}

			pendingResources.clear();
		}
	}

	void ResourceStateTracker::updateReferredResourceStates()
	{
		if (referredResources.size())
		{
			for (Resource::D3D12Resource::D3D12ResourceBase* const resource : referredResources)
			{
				resource->updateGlobalStates();

				resource->resetInternalState();

				resource->popFromReferredList();
			}

			referredResources.clear();
		}
	}

	void ResourceStateTracker::transitionResourceStates(ID3D12GraphicsCommandList6* const commandList)
	{
		if (transitionResources.size())
		{
			for (Resource::D3D12Resource::D3D12ResourceBase* const resource : transitionResources)
			{
				resource->transition(transitionBarriers, pendingResources);

				resource->resetTransitionState();

				resource->popFromTrackingList();
			}

			transitionResources.clear();
		}

		if (transitionBarriers.size())
		{
			commandList->ResourceBarrier(static_cast<uint32_t>(transitionBarriers.size()), transitionBarriers.data());

			transitionBarriers.clear();
		}
	}

	void ResourceStateTracker::pushResourceToTrackList(Resource::D3D12Resource::D3D12ResourceBase* const resource)
	{
		resource->pushToReferredList(referredResources);

		resource->pushToTrackingList(transitionResources);
	}
}
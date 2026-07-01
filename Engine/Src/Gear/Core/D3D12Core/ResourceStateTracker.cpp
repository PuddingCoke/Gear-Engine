#include<Gear/Core/D3D12Core/ResourceStateTracker.h>

namespace Gear::Core::D3D12Core
{
	void ResourceStateTracker::trackAndSetResourceState(D3D12Resource::Texture* const texture, const uint32_t mipslice, const uint32_t state)
	{
		if (texture->getStateTracking())
		{
			pushResourceToTrackList(texture);

			if (mipslice == D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS)
			{
				texture->setAllState(state);
			}
			else
			{
				texture->setMipSliceState(mipslice, state);
			}
		}
	}

	void ResourceStateTracker::trackAndSetResourceState(D3D12Resource::Buffer* const buffer, const uint32_t state)
	{
		if (buffer && buffer->getStateTracking())
		{
			pushResourceToTrackList(buffer);

			buffer->setState(state);
		}
	}

	void ResourceStateTracker::trackAndSetResourceState(D3D12Resource::VideoTexture* const videoTexture, const uint32_t state)
	{
		if (videoTexture->getStateTracking())
		{
			pushResourceToTrackList(videoTexture);

			videoTexture->setState(state);
		}
	}

	void ResourceStateTracker::flushPendingResources(std::vector<D3D12_RESOURCE_BARRIER>& outBarriers)
	{
		if (pendingResources.size())
		{
			for (D3D12Resource::D3D12ResourceBase* const resource : pendingResources)
			{
				resource->resolvePendingState(outBarriers);

				resource->resetPendingState();

				resource->popFromPendingList();
			}

			pendingResources.clear();
		}
	}

	void ResourceStateTracker::flushReferredResources()
	{
		if (referredResources.size())
		{
			for (D3D12Resource::D3D12ResourceBase* const resource : referredResources)
			{
				resource->updateGlobalStates();

				resource->resetInternalState();

				resource->popFromReferredList();
			}

			referredResources.clear();
		}
	}

	void ResourceStateTracker::flushTransitionResources()
	{
		if (transitionResources.size())
		{
			for (D3D12Resource::D3D12ResourceBase* const resource : transitionResources)
			{
				resource->transition(resourceBarriers, pendingResources);

				resource->resetTransitionState();

				resource->popFromTrackingList();
			}

			transitionResources.clear();
		}
	}

	void ResourceStateTracker::pushResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
	{
		resourceBarriers.push_back(barrier);
	}

	void ResourceStateTracker::pushResourceBarriers(const std::vector<D3D12_RESOURCE_BARRIER>& barriers)
	{
		for (const D3D12_RESOURCE_BARRIER& barrier : barriers)
		{
			pushResourceBarrier(barrier);
		}
	}

	bool ResourceStateTracker::hasPendingResource() const
	{
		return pendingResources.size();
	}

	void ResourceStateTracker::pushResourceToTrackList(D3D12Resource::D3D12ResourceBase* const resource)
	{
		resource->pushToReferredList(referredResources);

		resource->pushToTrackingList(transitionResources);
	}
}
#include<Gear/Core/Resource/D3D12Resource/D3D12ResourceBase.h>

namespace Gear::Core::Resource::D3D12Resource
{
	ID3D12Resource* D3D12ResourceBase::getResource() const
	{
		return resource.Get();
	}

	D3D12ResourceBase::~D3D12ResourceBase()
	{
	}

	void D3D12ResourceBase::updateGlobalStates()
	{
	}

	D3D12ResourceBase::D3D12ResourceBase(const ComPtr<ID3D12Resource>& resource, const bool stateTracking) :
		resource(resource), stateTracking(makeShared<bool>(stateTracking)), sharedResource(makeShared<bool>(false)),
		inReferredList(false), inTrackingList(false), inPendingList(false)
	{
	}

	D3D12ResourceBase::D3D12ResourceBase(const D3D12_HEAP_PROPERTIES properties, const D3D12_HEAP_FLAGS flags, const D3D12_RESOURCE_DESC desc,
		const bool stateTracking, const D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE* clearValues) :
		stateTracking(makeShared<bool>(stateTracking)), sharedResource(makeShared<bool>(false)),
		inReferredList(false), inTrackingList(false), inPendingList(false)
	{
		GraphicsDevice::get()->CreateCommittedResource(&properties, flags, &desc, initialState, clearValues, IID_PPV_ARGS(&resource));
	}

	D3D12ResourceBase::D3D12ResourceBase(D3D12ResourceBase& res) :
		resource(res.resource), stateTracking(res.stateTracking), sharedResource(res.sharedResource),
		inReferredList(false), inTrackingList(false), inPendingList(false)
	{
		*sharedResource = true;
	}

	D3D12_GPU_VIRTUAL_ADDRESS D3D12ResourceBase::getGPUAddress() const
	{
		return resource->GetGPUVirtualAddress();
	}

	void D3D12ResourceBase::setStateTracking(const bool state)
	{
		*stateTracking = state;
	}

	bool D3D12ResourceBase::getStateTracking() const
	{
		return *stateTracking;
	}

	bool D3D12ResourceBase::getSharedResource() const
	{
		return *sharedResource;
	}

	void D3D12ResourceBase::setName(LPCWSTR const name) const
	{
		resource->SetName(name);
	}

	bool D3D12ResourceBase::getInReferredList() const
	{
		return inReferredList;
	}

	bool D3D12ResourceBase::getInTrackingList() const
	{
		return inTrackingList;
	}

	bool D3D12ResourceBase::getInPendingList() const
	{
		return inPendingList;
	}

	void D3D12ResourceBase::popFromReferredList()
	{
		inReferredList = false;
	}

	void D3D12ResourceBase::popFromTrackingList()
	{
		inTrackingList = false;
	}

	void D3D12ResourceBase::popFromPendingList()
	{
		inPendingList = false;
	}

	void D3D12ResourceBase::pushToPendingList(std::vector<D3D12ResourceBase*>& pendingList)
	{
		if (!getInPendingList())
		{
			pendingList.push_back(this);

			inPendingList = true;
		}
	}

	void D3D12ResourceBase::pushToReferredList(std::vector<D3D12ResourceBase*>& referredList)
	{
		//只为需要状态追踪的共享资源更新它的全局状态
		if (getSharedResource() && !getInReferredList())
		{
			referredList.push_back(this);

			inReferredList = true;
		}
	}

	void D3D12ResourceBase::pushToTrackingList(std::vector<D3D12ResourceBase*>& trackingList)
	{
		if (!getInTrackingList())
		{
			trackingList.push_back(this);

			inTrackingList = true;
		}
	}

	void D3D12ResourceBase::transition(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers, std::vector<D3D12ResourceBase*>& pendingResources)
	{
	}

	void D3D12ResourceBase::resolvePendingState(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers)
	{
	}

	void D3D12ResourceBase::resetInternalState()
	{
	}

	void D3D12ResourceBase::resetTransitionState()
	{
	}

	void D3D12ResourceBase::resetPendingState()
	{
	}
}

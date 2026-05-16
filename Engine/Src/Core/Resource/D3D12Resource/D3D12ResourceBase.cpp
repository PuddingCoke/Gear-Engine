#include<Gear/Core/Resource/D3D12Resource/D3D12ResourceBase.h>

ID3D12Resource* Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::getResource() const
{
	return resource.Get();
}

Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::~D3D12ResourceBase()
{
}

Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::D3D12ResourceBase(const ComPtr<ID3D12Resource>& resource, const bool stateTracking) :
	resource(resource), stateTracking(std::make_shared<bool>(stateTracking)), sharedResource(std::make_shared<bool>(false)),
	inReferredList(false), inTrackingList(false)
{
}

Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::D3D12ResourceBase(const D3D12_HEAP_PROPERTIES properties, const D3D12_HEAP_FLAGS flags, const D3D12_RESOURCE_DESC desc,
	const bool stateTracking, const D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE* clearValues) :
	stateTracking(std::make_shared<bool>(stateTracking)), sharedResource(std::make_shared<bool>(false)),
	inReferredList(false), inTrackingList(false)
{
	Gear::Core::GraphicsDevice::get()->CreateCommittedResource(&properties, flags, &desc, initialState, clearValues, IID_PPV_ARGS(&resource));
}

Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::D3D12ResourceBase(D3D12ResourceBase& res) :
	resource(res.resource), stateTracking(res.stateTracking), sharedResource(res.sharedResource),
	inReferredList(false), inTrackingList(false)
{
	*sharedResource = true;
}

D3D12_GPU_VIRTUAL_ADDRESS Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::getGPUAddress() const
{
	return resource->GetGPUVirtualAddress();
}

void Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::setStateTracking(const bool state)
{
	*stateTracking = state;
}

bool Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::getStateTracking() const
{
	return *stateTracking;
}

bool Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::getSharedResource() const
{
	return *sharedResource;
}

void Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::setName(LPCWSTR const name) const
{
	resource->SetName(name);
}

bool Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::getInReferredList() const
{
	return inReferredList;
}

bool Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::getInTrackingList() const
{
	return inTrackingList;
}

void Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::pushToReferredList(std::vector<D3D12ResourceBase*>& referredList)
{
	//只为需要状态追踪的共享资源更新它的全局状态
	if (getSharedResource() && !getInReferredList())
	{
		referredList.push_back(this);

		inReferredList = true;
	}
}

void Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::popFromReferredList()
{
	inReferredList = false;
}

void Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::pushToTrackingList()
{
	inTrackingList = true;
}

void Gear::Core::Resource::D3D12Resource::D3D12ResourceBase::popFromTrackingList()
{
	inTrackingList = false;
}

#include<Gear/Core/D3D12Resource/Buffer.h>

#include<Gear/Utils/Math.h>

namespace Gear::Core::D3D12Resource
{
	Buffer::Buffer(const uint64_t size, const bool stateTracking, const D3D12_RESOURCE_FLAGS resFlags, const uint32_t initialState) :
		D3D12ResourceBase(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, CD3DX12_RESOURCE_DESC::Buffer(size, resFlags), stateTracking, static_cast<D3D12_RESOURCE_STATES>(initialState), nullptr),
		size(size),
		globalState(makeShared<uint32_t>(initialState)),
		internalState(initialState),
		transitionState(D3D12_RESOURCE_STATE_UNKNOWN),
		pendingState(D3D12_RESOURCE_STATE_UNKNOWN)
	{
	}

	Buffer::Buffer(const ComPtr<ID3D12Resource>& buffer, const bool stateTracking, const uint32_t initialState) :
		D3D12ResourceBase(buffer, stateTracking),
		globalState(makeShared<uint32_t>(initialState)),
		internalState(initialState),
		transitionState(D3D12_RESOURCE_STATE_UNKNOWN),
		pendingState(D3D12_RESOURCE_STATE_UNKNOWN)
	{
		const D3D12_RESOURCE_DESC desc = buffer->GetDesc();

		size = desc.Width;
	}

	Buffer::Buffer(Buffer& buff) :
		D3D12ResourceBase(buff),
		size(buff.size),
		globalState(buff.globalState),
		internalState(D3D12_RESOURCE_STATE_UNKNOWN),
		transitionState(D3D12_RESOURCE_STATE_UNKNOWN),
		pendingState(D3D12_RESOURCE_STATE_UNKNOWN)
	{
		buff.resetInternalState();
	}

	Buffer::~Buffer()
	{
	}

	void Buffer::updateGlobalStates()
	{
		if (internalState != D3D12_RESOURCE_STATE_UNKNOWN)
		{
			*globalState = internalState;
		}
	}

	void Buffer::transition(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers, std::vector<D3D12ResourceBase*>& pendingResources)
	{
		if (internalState == D3D12_RESOURCE_STATE_UNKNOWN)
		{
			pendingState = transitionState;

			internalState = transitionState;

			pushToPendingList(pendingResources);
		}
		else if (!Utils::Math::bitFlagSubset(internalState, transitionState))
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = getResource();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(internalState);
			barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(transitionState);

			resourceBarriers.push_back(barrier);

			internalState = transitionState;
		}
		else if (internalState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && transitionState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));
		}
	}

	void Buffer::resolvePendingState(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers)
	{
		if (*globalState != pendingState)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = getResource();
			barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(*globalState);
			barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(pendingState);
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			resourceBarriers.push_back(barrier);
		}
		else if (*globalState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && pendingState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			resourceBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(getResource()));
		}
	}

	void Buffer::resetInternalState()
	{
		internalState = D3D12_RESOURCE_STATE_UNKNOWN;
	}

	void Buffer::resetTransitionState()
	{
		transitionState = D3D12_RESOURCE_STATE_UNKNOWN;
	}

	void Buffer::resetPendingState()
	{
		pendingState = D3D12_RESOURCE_STATE_UNKNOWN;
	}

	void Buffer::setState(const uint32_t state)
	{
		if (transitionState == D3D12_RESOURCE_STATE_UNKNOWN)
		{
			transitionState = state;
		}
		else
		{
			transitionState = transitionState | state;
		}
	}

	uint64_t Buffer::getSize() const
	{
		return size;
	}
}

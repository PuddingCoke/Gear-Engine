#pragma once

#ifndef _GEAR_CORE_RESOURCE_D3D12RESOURCE_BUFFER_H_
#define _GEAR_CORE_RESOURCE_D3D12RESOURCE_BUFFER_H_

#include"D3D12ResourceBase.h"

#include<vector>

namespace Gear::Core::D3D12Resource
{
	CREATESAFETYPE(Buffer);

	class Buffer :public D3D12ResourceBase
	{
	public:

		Buffer() = delete;

		Buffer(const Buffer&) = delete;

		void operator=(const Buffer&) = delete;

		Buffer(const uint64_t size, const bool stateTracking, const D3D12_RESOURCE_FLAGS resFlags, const uint32_t initialState = D3D12_RESOURCE_STATE_COPY_DEST);

		Buffer(const ComPtr<ID3D12Resource>& buffer, const bool stateTracking, const uint32_t initialState);

		Buffer(Buffer&);

		virtual ~Buffer();

		void updateGlobalStates() override;

		void transition(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers, std::vector<D3D12ResourceBase*>& pendingResources) override;

		void resolvePendingState(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers) override;

		void resetInternalState() override;

		void resetTransitionState() override;

		void resetPendingState() override;

		void setState(const uint32_t state);

		uint32_t getState() const;

	private:

		SharedPtr<uint32_t> globalState;

		uint32_t internalState;

		uint32_t transitionState;

		uint32_t pendingState;

	};
}

#endif // !_GEAR_CORE_RESOURCE_D3D12RESOURCE_BUFFER_H_
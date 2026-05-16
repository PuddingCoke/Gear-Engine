#pragma once

#ifndef _GEAR_CORE_RESOURCE_D3D12RESOURCE_BUFFER_H_
#define _GEAR_CORE_RESOURCE_D3D12RESOURCE_BUFFER_H_

#include"D3D12ResourceBase.h"

#include<vector>

namespace Gear
{
	namespace Core
	{
		namespace Resource
		{
			namespace D3D12Resource
			{
				class Buffer;

				struct PendingBufferBarrier
				{
					Buffer* buffer;

					uint32_t afterState;
				};

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

					void resetInternalStates() override;

					void transition(std::vector<D3D12_RESOURCE_BARRIER>& transitionBarriers, std::vector<PendingBufferBarrier>& pendingBarriers);

					void solvePendingBarrier(std::vector<D3D12_RESOURCE_BARRIER>& transitionBarriers, const uint32_t targetState);

					void setState(const uint32_t state);

					uint32_t getState() const;

					void pushToTrackingList(std::vector<Buffer*>& trackingList);

				protected:

					void resetTransitionStates() override;

				private:

					std::shared_ptr<uint32_t> globalState;

					uint32_t internalState;

					uint32_t transitionState;

				};
			}
		}
	}
}

#endif // !_GEAR_CORE_RESOURCE_D3D12RESOURCE_BUFFER_H_
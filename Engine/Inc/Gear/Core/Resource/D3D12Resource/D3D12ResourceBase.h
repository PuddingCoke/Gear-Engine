#pragma once

#ifndef _GEAR_CORE_RESOURCE_D3D12RESOURCE_D3D12RESOURCEBASE_H_
#define _GEAR_CORE_RESOURCE_D3D12RESOURCE_D3D12RESOURCEBASE_H_

#include<Gear/Core/GraphicsDevice.h>

#include<Gear/Core/FMT.h>

#include<memory>

namespace Gear
{
	namespace Core
	{
		namespace Resource
		{
			namespace D3D12Resource
			{
				//用于内部状态、转变状态、全局状态
				constexpr uint32_t D3D12_RESOURCE_STATE_UNKNOWN = 0xFFFFFFFF;

				//用于待定资源屏障
				constexpr uint32_t D3D12_TRANSITION_ALL_MIPLEVELS = 0xFFFFFFFF;

				//a是否拥有b？
				constexpr bool bitFlagSubset(const uint32_t a, const uint32_t b)
				{
					return b && ((a & b) == b);
				}

				class D3D12ResourceBase
				{
				public:

					D3D12ResourceBase() = delete;

					D3D12ResourceBase(const D3D12ResourceBase&) = delete;

					void operator=(const D3D12ResourceBase&) = delete;

					D3D12ResourceBase(const ComPtr<ID3D12Resource>& resource, const bool stateTracking);

					D3D12ResourceBase(const D3D12_HEAP_PROPERTIES properties, const D3D12_HEAP_FLAGS flags, const D3D12_RESOURCE_DESC desc,
						const bool stateTracking, const D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE* clearValues);

					D3D12ResourceBase(D3D12ResourceBase&);

					virtual ~D3D12ResourceBase();

					virtual void updateGlobalStates() = 0;

					virtual void resetInternalStates() = 0;

					ID3D12Resource* getResource() const;

					D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;

					void setStateTracking(const bool state);

					bool getStateTracking() const;

					bool getSharedResource() const;

					void setName(LPCWSTR const name) const;

					bool getInReferredList() const;

					bool getInTrackingList() const;

					void pushToReferredList(std::vector<D3D12ResourceBase*>& referredList);

					void popFromReferredList();

					void pushToTrackingList();

					void popFromTrackingList();

				protected:

					virtual void resetTransitionStates() = 0;

				private:

					ComPtr<ID3D12Resource> resource;

					std::shared_ptr<bool> stateTracking;

					std::shared_ptr<bool> sharedResource;

					bool inReferredList;

					bool inTrackingList;

				};
			}
		}
	}
}

#endif // !_GEAR_CORE_RESOURCE_D3D12RESOURCE_D3D12RESOURCEBASE_H_

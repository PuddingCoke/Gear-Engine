#pragma once

#ifndef _GEAR_CORE_RESOURCE_D3D12RESOURCE_TEXTURE_H_
#define _GEAR_CORE_RESOURCE_D3D12RESOURCE_TEXTURE_H_

#include"D3D12ResourceBase.h"

#include<vector>

#include<algorithm>

namespace Gear
{
	namespace Core
	{
		namespace Resource
		{
			namespace D3D12Resource
			{
				class Texture;

				struct PendingTextureBarrier
				{
					Texture* texture;

					uint32_t mipSlice;

					uint32_t afterState;
				};

				class Texture :public D3D12ResourceBase
				{
				public:

					Texture() = delete;

					Texture(const Texture&) = delete;

					void operator=(const Texture&) = delete;

					Texture(const uint32_t width, const uint32_t height, const DXGI_FORMAT format, const uint32_t arraySize, const uint32_t mipLevels, const bool stateTracking, const D3D12_RESOURCE_FLAGS resFlags, const D3D12_CLEAR_VALUE* const clearValue = nullptr);

					Texture(const ComPtr<ID3D12Resource>& texture, const bool stateTracking, const uint32_t initialState);

					Texture(Texture&);

					virtual ~Texture();

					void updateGlobalStates() override;

					void resetInternalStates() override;

					void transition(std::vector<D3D12_RESOURCE_BARRIER>& transitionBarriers, std::vector<PendingTextureBarrier>& pendingBarriers);

					void solvePendingBarrier(std::vector<D3D12_RESOURCE_BARRIER>& transitionBarriers, const uint32_t targetMipSlice, const uint32_t targetState);

					uint32_t getWidth() const;

					uint32_t getHeight() const;

					uint32_t getArraySize() const;

					uint32_t getMipLevels() const;

					DXGI_FORMAT getFormat() const;

					void setAllState(const uint32_t state);

					void setMipSliceState(const uint32_t mipSlice, const uint32_t state);

					uint32_t getAllState() const;

					uint32_t getMipSliceState(const uint32_t mipSlice) const;

					void pushToTrackingList(std::vector<Texture*>& trackingList);

				protected:

					void resetTransitionStates() override;

				private:

					uint32_t width;

					uint32_t height;

					uint32_t arraySize;

					uint32_t mipLevels;

					DXGI_FORMAT format;

					struct States
					{
						States() = delete;

						States(const States&) = delete;

						void operator=(const States&) = delete;

						explicit States(const uint32_t initialState, const uint32_t mipLevels);

						~States();

						void set(const uint32_t state);

						void combine(const uint32_t state);

						void reset();

						bool allOfEqual(const uint32_t state) const;

						template<typename Func>
						void forEach(const Func& func) const;

						const uint32_t mipLevels;

						uint32_t allState;

						uint32_t* mipLevelStates;
					};

					std::shared_ptr<States> globalState;

					States* internalState;

					States* transitionState;

				};
			}
		}
	}
}

#endif // !_GEAR_CORE_RESOURCE_D3D12RESOURCE_TEXTURE_H_

template<typename Func>
inline void Gear::Core::Resource::D3D12Resource::Texture::States::forEach(const Func& func) const
{
	std::for_each(mipLevelStates, mipLevelStates + mipLevels, func);
}

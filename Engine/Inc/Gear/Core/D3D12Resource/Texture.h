#pragma once

#ifndef _GEAR_CORE_D3D12RESOURCE_TEXTURE_H_
#define _GEAR_CORE_D3D12RESOURCE_TEXTURE_H_

#include"D3D12ResourceBase.h"

namespace Gear::Core::D3D12Resource
{
	CREATESAFETYPE(Texture);

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

		void transition(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers, std::vector<D3D12ResourceBase*>& pendingResources) override;

		void resolvePendingState(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers) override;

		void resetInternalState() override;

		void resetTransitionState() override;

		void resetPendingState() override;

		uint32_t getWidth() const;

		uint32_t getHeight() const;

		uint32_t getArraySize() const;

		uint32_t getMipLevels() const;

		DXGI_FORMAT getFormat() const;

		void setAllState(const uint32_t state);

		void setMipSliceState(const uint32_t mipSlice, const uint32_t state);

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

			//直接设置状态
			void set(const uint32_t state) noexcept;

			//不考虑D3D12_RESOURCE_STATE_UNKNOWN的情况
			void combineSimple(const uint32_t state) noexcept;

			//考虑D3D12_RESOURCE_STATE_UNKNOWN的情况
			void combine(const uint32_t state) noexcept;

			//重置状态为D3D12_RESOURCE_STATE_UNKNOWN
			void reset() noexcept;

			//检查各个miplevel的状态是否都为state状态
			bool allOfEqual(const uint32_t state) const noexcept;

			const uint32_t mipLevels;

			uint32_t allState;

			UniquePtr<uint32_t[]> mipLevelStates;
		};

		SharedPtr<States> globalState;

		UniquePtr<States> internalState;

		UniquePtr<States> transitionState;

		UniquePtr<States> pendingState;

	};
}

#endif // !_GEAR_CORE_D3D12RESOURCE_TEXTURE_H_

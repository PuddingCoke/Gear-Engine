#pragma once

#ifndef _GEAR_CORE_D3D12RESOURCE_VIDEOTEXTURE_H_
#define _GEAR_CORE_D3D12RESOURCE_VIDEOTEXTURE_H_

#include"D3D12ResourceBase.h"

#include<D3D12Headers/d3d12video.h>

namespace Gear::Core::D3D12Resource
{
	CREATESAFETYPE(VideoTexture);

	class VideoTexture :public D3D12ResourceBase
	{
	public:

		VideoTexture() = delete;

		VideoTexture(const VideoTexture&) = delete;

		void operator=(const VideoTexture&) = delete;

		VideoTexture(const uint32_t width, const uint32_t height, const DXGI_FORMAT format, const D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE);

		//应该不需要深拷贝？
		//VideoTexture(VideoTexture&);

		virtual ~VideoTexture() = default;

		void updateGlobalStates() override;

		void transition(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers, std::vector<D3D12ResourceBase*>& pendingResources) override;

		void resolvePendingState(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers) override;

		void resetInternalState() override;

		void resetTransitionState() override;

		void resetPendingState() override;

		uint32_t getWidth() const;

		uint32_t getHeight() const;

		DXGI_FORMAT getFormat() const;

		void setState(const uint32_t state);

	private:

		uint32_t width;

		uint32_t height;

		DXGI_FORMAT format;

		SharedPtr<uint32_t> globalState;

		uint32_t internalState;

		uint32_t transitionState;

		uint32_t pendingState;

	};
}

#endif // !_GEAR_CORE_D3D12RESOURCE_VIDEOTEXTURE_H_

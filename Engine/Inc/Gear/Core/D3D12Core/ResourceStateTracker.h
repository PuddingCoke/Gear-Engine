#pragma once

#ifndef _GEAR_CORE_D3D12CORE_RESOURCESTATETRACKER_H_
#define _GEAR_CORE_D3D12CORE_RESOURCESTATETRACKER_H_

#include<Gear/Core/D3D12Resource/Buffer.h>

#include<Gear/Core/D3D12Resource/Texture.h>

#include<Gear/Core/D3D12Resource/VideoTexture.h>

namespace Gear::Core::D3D12Core
{
	class ResourceStateTracker
	{
	public:

		//以下的方法会追踪并设置资源状态

		void trackAndSetResourceState(D3D12Resource::Texture* const texture, const uint32_t mipslice, const uint32_t state);

		void trackAndSetResourceState(D3D12Resource::Buffer* const buffer, const uint32_t state);

		void trackAndSetResourceState(D3D12Resource::VideoTexture* const videoTexture, const uint32_t state);

		//清空BEFORE STATE待定的资源
		void flushPendingResources(std::vector<D3D12_RESOURCE_BARRIER>& outBarriers);

		//清空使用过的共享资源
		void flushReferredResources();

		//清空需要状态转变的资源
		void flushTransitionResources();

		//手动插入资源屏障
		void pushResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

		//手动插入资源屏障
		void pushResourceBarriers(const std::vector<D3D12_RESOURCE_BARRIER>& barriers);

		//是否有待定资源
		bool hasPendingResource() const;

	protected:

		//暂存资源屏障
		std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

	private:

		void pushResourceToTrackList(D3D12Resource::D3D12ResourceBase* const resource);

		//暂存使用过的共享且需要状态追踪的资源
		std::vector<D3D12Resource::D3D12ResourceBase*> referredResources;

		//暂存需要状态转变的资源
		std::vector<D3D12Resource::D3D12ResourceBase*> transitionResources;

		//暂存BEFORE STATE待定的资源，需要主渲染线程帮忙解决
		std::vector<D3D12Resource::D3D12ResourceBase*> pendingResources;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_RESOURCESTATETRACKER_H_

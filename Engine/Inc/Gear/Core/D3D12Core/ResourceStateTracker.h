#pragma once

#ifndef _GEAR_CORE_D3D12CORE_RESOURCESTATETRACKER_H_
#define _GEAR_CORE_D3D12CORE_RESOURCESTATETRACKER_H_

#include<Gear/Core/Resource/D3D12Resource/Buffer.h>

#include<Gear/Core/Resource/D3D12Resource/Texture.h>

namespace Gear::Core::D3D12Core
{
	class ResourceStateTracker
	{
	public:

		void trackAndSetResourceState(Resource::D3D12Resource::Texture* const texture, const uint32_t mipslice, const uint32_t state);

		void trackAndSetResourceState(Resource::D3D12Resource::Buffer* const buffer, const uint32_t state);

		void resolvePendingResourceStates(std::vector<D3D12_RESOURCE_BARRIER>& outBarriers);

		void updateReferredResourceStates();

	protected:

		void transitionResourceStates(ID3D12GraphicsCommandList6* const commandList);

	private:

		void pushResourceToTrackList(Resource::D3D12Resource::D3D12ResourceBase* const resource);

		//暂存使用过的共享且需要状态追踪的资源
		std::vector<Resource::D3D12Resource::D3D12ResourceBase*> referredResources;

		//暂存需要状态转变的资源
		std::vector<Resource::D3D12Resource::D3D12ResourceBase*> transitionResources;

		//暂存BEFORE STATE待定的资源，需要主渲染线程帮忙解决
		std::vector<Resource::D3D12Resource::D3D12ResourceBase*> pendingResources;

		//暂存资源屏障
		std::vector<D3D12_RESOURCE_BARRIER> transitionBarriers;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_RESOURCESTATETRACKER_H_

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

		void solvePendingBarriers(std::vector<D3D12_RESOURCE_BARRIER>& outBarriers);

		void updateReferredSharedResourceStates();

	protected:

		void transitionResources(ID3D12GraphicsCommandList6* const commandList);

	private:

		void pushResourceToTrackList(Resource::D3D12Resource::Texture* const texture);

		void pushResourceToTrackList(Resource::D3D12Resource::Buffer* const buffer);

		//记录使用过的共享且需要状态追踪的资源
		std::vector<Resource::D3D12Resource::D3D12ResourceBase*> referredResources;

		//暂存需要状态转变的缓冲
		std::vector<Resource::D3D12Resource::Buffer*> transitionBuffers;

		//暂存需要状态转变的纹理
		std::vector<Resource::D3D12Resource::Texture*> transitionTextures;

		//暂存资源屏障
		std::vector<D3D12_RESOURCE_BARRIER> transitionBarriers;

		//以下是待定资源屏障，对于共享且需要状态追踪的资源来说它的内部状态是未知的，即STATE_BEFORE是未知的
		//因此需要待定资源屏障，并让主渲染线程解决这个问题

		std::vector<Resource::D3D12Resource::PendingBufferBarrier> pendingBufferBarrier;

		std::vector<Resource::D3D12Resource::PendingTextureBarrier> pendingTextureBarrier;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_RESOURCESTATETRACKER_H_

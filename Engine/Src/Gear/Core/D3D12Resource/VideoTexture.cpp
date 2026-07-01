#include<Gear/Core/D3D12Resource/VideoTexture.h>

#include<Gear/Utils/Math.h>

namespace Gear::Core::D3D12Resource
{
	VideoTexture::VideoTexture(const uint32_t width, const uint32_t height, const DXGI_FORMAT format, const D3D12_RESOURCE_FLAGS resFlags) :
		D3D12ResourceBase(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1, 1, 0, resFlags), true, D3D12_RESOURCE_STATE_COPY_DEST, nullptr),
		width(width),
		height(height),
		format(format),
		globalState(makeShared<uint32_t>(D3D12_RESOURCE_STATE_COPY_DEST)),
		internalState(D3D12_RESOURCE_STATE_COPY_DEST),
		transitionState(D3D12_RESOURCE_STATE_UNKNOWN),
		pendingState(D3D12_RESOURCE_STATE_UNKNOWN)
	{
		if (format != DXGI_FORMAT_NV12 &&
			format != DXGI_FORMAT_P010 &&
			format != DXGI_FORMAT_P016 &&
			format != DXGI_FORMAT_NV11 &&
			format != DXGI_FORMAT_YUY2 &&
			format != DXGI_FORMAT_AYUV &&
			format != DXGI_FORMAT_Y210 &&
			format != DXGI_FORMAT_Y410 &&
			format != DXGI_FORMAT_Y416 &&
			format != DXGI_FORMAT_Y216 &&
			format != DXGI_FORMAT_420_OPAQUE)
		{
			LOGERROR(L"侦测到输入格式为非视频专用的纹理格式");
		}
	}

	void VideoTexture::updateGlobalStates()
	{
		if (internalState != D3D12_RESOURCE_STATE_UNKNOWN)
		{
			*globalState = internalState;
		}
	}

	void VideoTexture::transition(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers, std::vector<D3D12ResourceBase*>& pendingResources)
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
	}

	void VideoTexture::resolvePendingState(std::vector<D3D12_RESOURCE_BARRIER>& resourceBarriers)
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
	}

	void VideoTexture::resetInternalState()
	{
		internalState = D3D12_RESOURCE_STATE_UNKNOWN;
	}

	void VideoTexture::resetTransitionState()
	{
		transitionState = D3D12_RESOURCE_STATE_UNKNOWN;
	}

	void VideoTexture::resetPendingState()
	{
		pendingState = D3D12_RESOURCE_STATE_UNKNOWN;
	}

	uint32_t VideoTexture::getWidth() const
	{
		return width;
	}

	uint32_t VideoTexture::getHeight() const
	{
		return height;
	}

	DXGI_FORMAT VideoTexture::getFormat() const
	{
		return format;
	}

	void VideoTexture::setState(const uint32_t state)
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
}
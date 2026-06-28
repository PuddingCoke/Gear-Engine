#pragma once

#ifndef _GEAR_CORE_RENDERENGINE_INTERNAL_H_
#define _GEAR_CORE_RENDERENGINE_INTERNAL_H_

#include<Gear/Core/D3D12Resource/ReadbackHeap.h>

namespace Gear::Core::RenderEngine::Internal
{
	void initialize(const uint32_t width, const uint32_t height, const HWND hwnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface);

	void release();

	struct InitializeToken
	{
		InitializeToken(const uint32_t width, const uint32_t height, const HWND hwnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface)
		{
			initialize(width, height, hwnd, useSwapChainBuffer, initializeImGuiSurface);
		}

		~InitializeToken()
		{
			release();
		}
	};

	void waitForCurrentFrame();

	void waitForNextFrame();

	void beginFrame();

	void endFrame();

	void processCommandLists();

	void present();

	void setDeltaTime(const float deltaTime);

	void updateTimeElapsed();

	void saveBackBuffer(D3D12Resource::ReadbackHeap* const readbackHeap);

	void setDefRenderTexture();

	void setRenderTexture(D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle);

	void initializeResources();
}

#endif // !_GEAR_CORE_RENDERENGINE_INTERNAL_H_

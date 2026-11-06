#pragma once

#ifndef _GEAR_CORE_RENDERENGINE_INTERNAL_H_
#define _GEAR_CORE_RENDERENGINE_INTERNAL_H_

#include <Gear/Core/D3D12Resource/ReadbackHeap.h>

namespace Gear::Core::RenderEngine::Internal
{
	void initialize(const uint32_t width,
	                const uint32_t height,
	                const HWND hWnd,
	                const bool useSwapChainBuffer,
	                const bool initializeImGuiSurface);

	void release();

	struct InitializeToken
	{
		InitializeToken(const uint32_t width,
		                const uint32_t height,
		                const HWND hWnd,
		                const bool useSwapChainBuffer,
		                const bool initializeImGuiSurface)
		{
			initialize(width, height, hWnd, useSwapChainBuffer, initializeImGuiSurface);
		}

		~InitializeToken()
		{
			release();
		}
	};

	//更新帧索引
	void updateFrameIndex();

	//等待可销毁
	void waitDestroyable();

	//等待帧GPU完成
	void waitFrameGPUComplete();

	//等待帧CPU可复用
	void waitFrameCPUReusable();

	//开始一帧
	//后备缓冲被转换到STATE_RENDER_TARGET
	void beginFrame();

	//结束这一帧，调用ExecuteCommandLists
	//后备缓冲被转换到STATE_PRESENT
	void endFrame();

	//呈现后备缓冲
	void present();

	//设置同步间隔
	void setSyncInterval(const int32_t syncInterval);

	//将后备缓冲拷贝到指定读回堆上
	void saveBackBuffer(D3D12Resource::ReadbackHeapPtr& readbackHeap,
	                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& backBufferFootprint);

	//设置后备缓冲为渲染纹理
	void setDefRenderTexture();

	//设置指定纹理为渲染纹理
	void setRenderTexture(D3D12Resource::Texture* const renderTexture,
	                      const D3D12_CPU_DESCRIPTOR_HANDLE handle);

	//初始化资源（会等待GPU完成任务）
	void initializeResources();

	//设置变化的时间
	void setDeltaTime(const float deltaTime);

	//更新总流逝时间
	void updateTimeElapsed();

	//设置帧率
	void setFrameRate(const float frameRate);

	//使总帧数渲染自增1
	void renderedFrameCountInc();
}

#endif // !_GEAR_CORE_RENDERENGINE_INTERNAL_H_

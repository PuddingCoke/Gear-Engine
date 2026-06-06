#pragma once

#ifndef _GEAR_CORE_RENDERTASK_H_
#define _GEAR_CORE_RENDERTASK_H_

#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/ResourceManager.h>

namespace Gear::Core
{
	class RenderTask
	{
	public:

		RenderTask(const RenderTask&) = delete;

		void operator=(const RenderTask&) = delete;

		RenderTask();

		virtual ~RenderTask();

		void beginTask();

		bool waitTask();

		D3D12Core::CommandList* getCommandList() const;

		virtual void imGUICall();

	protected:

		//把纹理绘制到后备缓冲上
		void blit(Resource::RenderTextureView& texture) const;

		virtual void recordCommand() = 0;

		UniquePtr<ResourceManager> resManager;

		//引用
		GraphicsContext* const context;

	private:

		friend class RenderThread;

		void workerLoop();

		bool taskCompleted;

		bool errorOccur;

		bool isRunning;

		std::mutex taskMutex;

		std::condition_variable taskCondition;

		UniquePtr<RenderThread> renderThread;

	};
}

#endif // !_GEAR_CORE_RENDERTASK_H_
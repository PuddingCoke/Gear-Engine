#pragma once

#ifndef _GEAR_CORE_RENDERTASK_H_
#define _GEAR_CORE_RENDERTASK_H_

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

		virtual void imGuiCall();

		std::string getErrorStr() const;

	protected:

		//把纹理绘制到后备缓冲上
		void blit(Resource::RenderTextureView& texture) const;

		virtual void recordCommand() = 0;

		ResourceManagerPtr resManager;

		//注意：只是引用
		GraphicsContext* const context;

	private:

		friend class RenderThread;

		//帧任务
		void frameTask();

		UniquePtr<RenderThread> renderThread;

	};
}

#endif // !_GEAR_CORE_RENDERTASK_H_
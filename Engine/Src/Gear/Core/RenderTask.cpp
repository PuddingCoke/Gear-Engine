#include<Gear/Core/RenderThread.h>

#include<Gear/Effect/BackBufferBlitEffect.h>

namespace Gear::Core
{
	RenderTask::RenderTask() :
		resManager(makeUnique<ResourceManager>()),
		context(resManager->getGraphicsContext()),
		renderThread(nullptr)
	{
		context->begin();
	}

	RenderTask::~RenderTask()
	{
	}

	void RenderTask::beginTask()
	{
		renderThread->beginTask();
	}

	bool RenderTask::waitTask()
	{
		return renderThread->waitTask();
	}

	D3D12Core::CommandList* RenderTask::getCommandList() const
	{
		return context->getCommandList();
	}

	void RenderTask::imGuiCall()
	{

	}

	std::string RenderTask::getErrorStr() const
	{
		return renderThread->getErrorStr();
	}

	void RenderTask::blit(Resource::RenderTextureView& texture) const
	{
		Effect::BackBufferBlitEffect::process(*context, texture);
	}

	void RenderTask::frameTask()
	{
		resManager->cleanTransientResources();

		context->begin();

		recordCommand();
	}
}

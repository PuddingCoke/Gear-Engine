#include <Gear/Core/D3D12Core/CommandQueue.h>

#include <Gear/Core/Graphics.h>

namespace Gear::Core::D3D12Core
{
	CommandQueue::CommandQueue(CommandList* const prepareCommandList) :
		prepareCommandList(prepareCommandList),
		commandQueueType(prepareCommandList->getType()),
		frameBufferFence(makeUnique<Fence>()),
		currentFrameBufferFenceValue(0ull),
		lastUsableCommandList(nullptr)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = commandQueueType;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;

		GraphicsDevice::get()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

		frameBufferFenceValues = makeUnique<uint64_t[]>(Graphics::getFrameBufferCount());

		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			frameBufferFenceValues[i] = 0;
		}
	}

	CommandQueue::~CommandQueue()
	{
	}

	ID3D12CommandQueue* CommandQueue::get() const
	{
		return commandQueue.Get();
	}

	void CommandQueue::setName(const wchar_t* const name)
	{
		commandQueue->SetName(name);
	}

	void CommandQueue::waitDestroyable()
	{
		signalFrameCPUReusable();

		waitFrameGPUComplete();
	}

	void CommandQueue::waitFrameCPUReusable()
	{
		//如果completedValue小于记录的fenceValue，那么要等待CPU可复用
		if (frameBufferFence->getCompletedValue() < frameBufferFenceValues[Graphics::getFrameIndex()])
		{
			frameBufferFence->waitValue(frameBufferFenceValues[Graphics::getFrameIndex()]);
		}
	}

	void CommandQueue::waitFrameGPUComplete()
	{
		frameBufferFence->waitValue(currentFrameBufferFenceValue);
	}

	void CommandQueue::begin()
	{
		prepareCommandList->open();

		recordCommandLists.push(prepareCommandList);

		lastUsableCommandList = nullptr;
	}

	void CommandQueue::submitCommandList(CommandList* const commandList)
	{
		std::lock_guard<std::mutex> lockGuard(submitCommandListMutex);

		CommandList* const helperCommandList = recordCommandLists.back();

		if (commandList->hasPendingResource())
		{
			resourceBarriers.clear();

			commandList->flushPendingResources(resourceBarriers);

			if (helperCommandList != prepareCommandList)
			{
				helperCommandList->resourceBarrier(static_cast<uint32_t>(resourceBarriers.size()),
				                                   resourceBarriers.data());
			}
			else
			{
				//尽量减少D3D12 API ResourceBarrier调用
				helperCommandList->pushResourceBarriers(resourceBarriers);
			}

			//有待定资源那么会需要更新资源的全局状态
			//因此最后会需要更新使用过的资源的全局状态
			commandList->flushReferredResources();
		}

		//不应该关闭准备命令列表或最后一个可用的命令列表
		//因为这两个命令列表是特殊的，可能会被用来执行一些特殊操作
		if (helperCommandList != lastUsableCommandList && helperCommandList != prepareCommandList)
		{
			helperCommandList->close();
		}

		//获取最后一个可用的命令列表
		if (commandList->getType() == commandQueueType)
		{
			if (lastUsableCommandList)
			{
				lastUsableCommandList->close();
			}

			lastUsableCommandList = commandList;
		}

		recordCommandLists.push(commandList);
	}

	void CommandQueue::processCommandLists()
	{
		recordCommandLists.front()->close();

		if (recordCommandLists.size() > 1)
		{
			recordCommandLists.back()->close();

			if (lastUsableCommandList != recordCommandLists.back())
			{
				lastUsableCommandList->close();
			}
		}

		id3d12CommandLists.clear();

		for (const CommandList* const commandList : recordCommandLists)
		{
			id3d12CommandLists.push(commandList->get());
		}

		recordCommandLists.clear();

		commandQueue->ExecuteCommandLists(static_cast<uint32_t>(id3d12CommandLists.size()),
		                                  id3d12CommandLists.data());

		signalFrameCPUReusable();
	}

	uint64_t CommandQueue::signal(Fence* const fence)
	{
		return fence->signal(commandQueue.Get());
	}

	void CommandQueue::wait(Fence* const fence, const uint64_t waitValue)
	{
		commandQueue->Wait(fence->get(), waitValue);
	}

	void CommandQueue::waitFor(CommandQueue* const queueWaitFor, Fence* const fence)
	{
		wait(fence, queueWaitFor->signal(fence));
	}

	CommandList* CommandQueue::getLastUsableCommandList() const
	{
		return lastUsableCommandList;
	}

	void CommandQueue::signalFrameCPUReusable()
	{
		//Signal后要记录自增后的fenceValue

		//用于等待GPU完成
		currentFrameBufferFenceValue = signal(frameBufferFence.get());

		//用于多重缓冲机制
		frameBufferFenceValues[Graphics::getFrameIndex()] = currentFrameBufferFenceValue;
	}
}

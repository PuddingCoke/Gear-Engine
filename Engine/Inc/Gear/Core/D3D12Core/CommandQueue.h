#pragma once

#ifndef _GEAR_CORE_D3D12CORE_COMMANDQUEUE_H_
#define _GEAR_CORE_D3D12CORE_COMMANDQUEUE_H_

#include <Gear/Utils/StaticVector.h>

#include "CommandList.h"

#include "Fence.h"

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(CommandQueue);

	class CommandQueue
	{
	public:

		CommandQueue(CommandList* const prepareCommandList);

		virtual ~CommandQueue();

		ID3D12CommandQueue* get() const;

		void setName(const wchar_t* const name);

		void waitDestroyable();

		void waitFrameCPUReusable();

		void waitFrameGPUComplete();

		void begin();

		void submitCommandList(CommandList* const commandList);

		void processCommandLists();

		uint64_t signal(Fence* const fence);

		void wait(Fence* const fence, const uint64_t waitValue);

		//等待另一个命令队列
		void waitFor(CommandQueue* const queueWaitFor, Fence* const fence);

		CommandList* getLastUsableCommandList() const;

	private:

		//用于N重缓冲
		void signalFrameCPUReusable();

		ComPtr<ID3D12CommandQueue> commandQueue;

		const D3D12_COMMAND_LIST_TYPE commandQueueType;

		FencePtr frameBufferFence;

		UniquePtr<uint64_t[]> frameBufferFenceValues;

		uint64_t currentFrameBufferFenceValue;

		static constexpr uint64_t recordCommandListsLength = 32ull;

		Utils::StaticVector<CommandList*, recordCommandListsLength> recordCommandLists;

		Utils::StaticVector<ID3D12CommandList*, recordCommandListsLength> id3d12CommandLists;

		std::mutex submitCommandListMutex;

		std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

		CommandList* prepareCommandList;

		CommandList* lastUsableCommandList;
	};
}

#endif // !_GEAR_CORE_D3D12CORE_COMMANDQUEUE_H_

#pragma once

#ifndef _GEAR_CORE_D3D12CORE_COMMANDLIST_H_
#define _GEAR_CORE_D3D12CORE_COMMANDLIST_H_

#include"ResourceStateTracker.h"

namespace Gear::Core::D3D12Core
{
	class CommandList :public ResourceStateTracker
	{
	public:

		CommandList() = delete;

		CommandList(const CommandList&) = delete;

		void operator=(const CommandList&) = delete;

		CommandList(const D3D12_COMMAND_LIST_TYPE type);

		virtual ~CommandList() = default;

		virtual void open() = 0;

		virtual void close() = 0;

		virtual void resourceBarrier(const uint32_t numBarriers, const D3D12_RESOURCE_BARRIER* const pBarriers) const = 0;

		ID3D12CommandList* get() const;

		D3D12_COMMAND_LIST_TYPE getType() const;

		//清空需要状态转变的资源并调用resourceBarrier
		void flushResourceBarriers();

	protected:

		void setAndCloseCommandList(ID3D12CommandList* const commandList);

		void resetCommandAllocator() const;

		ID3D12CommandAllocator* getCommandAllocator() const;

	private:

		UniquePtr<ComPtr<ID3D12CommandAllocator>[]> allocators;

		ID3D12CommandList* commandList;

		const D3D12_COMMAND_LIST_TYPE commandListType;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_COMMANDLIST_H_

#pragma once

#ifndef _GEAR_CORE_RESOURCE_COUNTERBUFFERVIEW_H_
#define _GEAR_CORE_RESOURCE_COUNTERBUFFERVIEW_H_

#include"D3D12Resource/Buffer.h"

#include"ResourceBase.h"

namespace Gear::Core::Resource
{
	CREATESAFETYPE(CounterBufferView);

	class CounterBufferView :public ResourceBase
	{
	public:

		CounterBufferView() = delete;

		CounterBufferView(const bool persistent);

		CounterBufferView(const CounterBufferView&);

		~CounterBufferView();

		D3D12Resource::ShaderResourceDesc getSRVIndex() const;

		D3D12Resource::ShaderResourceDesc getUAVIndex() const;

		D3D12Resource::ClearUAVDesc getClearUAVDesc() const;

		void copyDescriptors() override;

		D3D12Resource::Buffer* getBuffer() const;

	private:

		SharedPtr<uint32_t> srvIndex;

		SharedPtr<uint32_t> uavIndex;

		SharedPtr<D3D12_GPU_DESCRIPTOR_HANDLE> viewGPUHandle;

		SharedPtr<D3D12_CPU_DESCRIPTOR_HANDLE> viewCPUHandle;

		D3D12Resource::BufferPtr buffer;
	};
}

#endif // !_GEAR_CORE_RESOURCE_COUNTERBUFFERVIEW_H_
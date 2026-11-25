#pragma once

#ifndef _GEAR_RESOURCE_COUNTERBUFFERVIEW_H_
#define _GEAR_RESOURCE_COUNTERBUFFERVIEW_H_

#include <Gear/Core/D3D12Resource/Buffer.h>

#include "ResourceBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(CounterBufferView);

	class CounterBufferView : public ResourceBase
	{
	public:

		CounterBufferView();

		CounterBufferView(const CounterBufferView&);

		~CounterBufferView();

		ShaderResourceDesc getSRVIndex();

		ShaderResourceDesc getUAVIndex();

		UAVClearDesc getUAVClearDesc();

		D3D12Resource::Buffer* getBuffer() const;

	private:

		uint32_t srvIndex;

		uint32_t uavIndex;

		D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle;

		D3D12Resource::BufferPtr buffer;
	};
}

#endif // !_GEAR_RESOURCE_COUNTERBUFFERVIEW_H_

#pragma once

#ifndef _GEAR_CORE_LOCALDESCRIPTORHEAP_H_
#define _GEAR_CORE_LOCALDESCRIPTORHEAP_H_

#include<Gear/Core/D3D12Core/DescriptorHeap.h>

namespace Gear
{
	namespace Core
	{
		namespace LocalDescriptorHeap
		{

			D3D12Core::DescriptorHeap* getStagingResourceHeap();

			D3D12Core::DescriptorHeap* getRenderTargetHeap();

			D3D12Core::DescriptorHeap* getDepthStencilHeap();

		}
	}
}

#endif // !_GEAR_CORE_LOCALDESCRIPTORHEAP_H_

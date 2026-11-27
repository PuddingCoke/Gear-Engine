#pragma once

#ifndef _GEAR_CORE_DYNAMICCBUFFERMANAGER_H_
#define _GEAR_CORE_DYNAMICCBUFFERMANAGER_H_

#include <Gear/Utils/Memory.h>

#include <directx/d3dx12.h>

namespace Gear::Core::DynamicCBufferManager
{

	//持久驻留的位置
	class AvailableLocation
	{
	public:

		AvailableLocation(const uint32_t sizeParam);

		~AvailableLocation();

		uint32_t getRegionIndex() const;
		
		uint32_t getSubRegionIndex() const;

		uint32_t getSize() const;

		D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;

		uint32_t getDescriptorIndex() const;

		void* getUploadPtr() const;

	private:

		uint32_t checkAndGetRegionIndex(const uint32_t sizeParam);

		//区域索引用于释放资源
		const uint32_t regionIndex;

		//子区域索引用于更新内容和释放资源
		const uint32_t subRegionIndex;

		//缓冲区实际大小
		const uint32_t size;

		void* const uploadPtr;

	};

	using AvailableLocationPtr = SharedPtr<AvailableLocation>;

}

#endif // !_GEAR_CORE_DYNAMICCBUFFERMANAGER_H_

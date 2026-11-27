#pragma once

#ifndef _GEAR_RESOURCE_DYNAMICINDEXTABLE_H_
#define _GEAR_RESOURCE_DYNAMICINDEXTABLE_H_

#include "IndexTable.h"

#include "DynamicCBuffer.h"

namespace Gear::Resource
{
	CREATESAFETYPE(DynamicIndexTable);

	//每帧都需要用到，位置与索引由DynamicCBufferManager保证稳定
	//DynamicCBuffer自带一份影子缓冲，因此索引表不需要再准备影子数据
	class DynamicIndexTable : public IndexTable
	{
	public:

		template<size_t N>
		DynamicIndexTable(const ShaderResourceDesc(&descs)[N]);

		DynamicIndexTable(const DynamicIndexTable&) = delete;

		void operator=(const DynamicIndexTable&) = delete;

		~DynamicIndexTable();

		//更新资源描述，是否真的需要刷新缓冲区由updateIndices决定
		template<size_t N>
		void updateShaderResourceDescs(const ShaderResourceDesc(&descs)[N]);

		Core::D3D12Resource::Buffer* getBuffer() const override;

		D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const override;

		UpdateStruct getUpdateStruct() const override;

		bool updateIndices() override;

	private:

		DynamicCBufferPtr cbuffer;
	};

	template<size_t N>
	inline DynamicIndexTable::DynamicIndexTable(const ShaderResourceDesc(&descs)[N]) :
		cbuffer(makeUnique<DynamicCBuffer>(byteSize))
	{
		if (cbuffer->getSize() != byteSize)
		{
			THROWLOG(LOGERROR() << "索引表的动态常量缓冲大小必须为" << byteSize << "字节！");
		}

		initializeShaderResourceDescs(descs);

		//影子缓冲的内容此时是未定义的，因此第一次使用前必须写入一次
		setUserUpdate(true);
	}

	template<size_t N>
	inline void DynamicIndexTable::updateShaderResourceDescs(const ShaderResourceDesc(&descs)[N])
	{
		initializeShaderResourceDescs(descs);

		setUserUpdate(true);
	}
}

#endif // !_GEAR_RESOURCE_DYNAMICINDEXTABLE_H_

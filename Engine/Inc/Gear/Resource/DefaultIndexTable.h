#pragma once

#ifndef _GEAR_RESOURCE_DEFAULTINDEXTABLE_H_
#define _GEAR_RESOURCE_DEFAULTINDEXTABLE_H_

#include "IndexTable.h"

#include "DefaultCBuffer.h"

namespace Gear::Resource
{
	CREATESAFETYPE(DefaultIndexTable);

	//由用户指定是否需要更新，影子数据保存着索引表的权威内容
	class DefaultIndexTable : public IndexTable
	{
	public:

		template<size_t N>
		DefaultIndexTable(DefaultCBufferPtr cbufferPtr, const ShaderResourceDesc(&descs)[N]);

		DefaultIndexTable(const DefaultIndexTable&) = delete;

		void operator=(const DefaultIndexTable&) = delete;

		~DefaultIndexTable();

		//更新资源描述，是否真的需要刷新缓冲区由updateIndices决定
		template<size_t N>
		void updateShaderResourceDescs(const ShaderResourceDesc(&descs)[N]);

		Core::D3D12Resource::Buffer* getBuffer() const override;

		D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const override;

		UpdateStruct getUpdateStruct() const override;

		bool updateIndices() override;

	private:

		DefaultCBufferPtr cbuffer;

		//索引表的影子数据，同时也是判断索引是否真的变了的依据
		uint32_t shadowIndices[maxIndexNum];
	};

	template<size_t N>
	inline DefaultIndexTable::DefaultIndexTable(DefaultCBufferPtr cbufferPtr,
	                                            const ShaderResourceDesc(&descs)[N]) :
		cbuffer(std::move(cbufferPtr))
	{
		if (!cbuffer)
		{
			THROWLOG(LOGERROR() << "默认类型的索引表必须持有一个常量缓冲！");
		}

		if (cbuffer->getBuffer()->getSize() != byteSize)
		{
			THROWLOG(LOGERROR() << "索引表的常量缓冲大小必须为" << byteSize << "字节！");
		}

		for (uint32_t i = 0; i < maxIndexNum; i++)
		{
			shadowIndices[i] = UINT32_MAX;
		}

		initializeShaderResourceDescs(descs);

		//缓冲区的内容此时是未定义的，因此第一次使用前必须上传一次
		setUserUpdate(true);
	}

	template<size_t N>
	inline void DefaultIndexTable::updateShaderResourceDescs(const ShaderResourceDesc(&descs)[N])
	{
		initializeShaderResourceDescs(descs);

		setUserUpdate(true);
	}
}

#endif // !_GEAR_RESOURCE_DEFAULTINDEXTABLE_H_

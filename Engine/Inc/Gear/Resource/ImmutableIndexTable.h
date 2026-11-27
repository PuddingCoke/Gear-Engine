#pragma once

#ifndef _GEAR_RESOURCE_IMMUTABLEINDEXTABLE_H_
#define _GEAR_RESOURCE_IMMUTABLEINDEXTABLE_H_

#include "IndexTable.h"

#include "ImmutableCBuffer.h"

namespace Gear::Resource
{
	CREATESAFETYPE(ImmutableIndexTable);

	//每帧都不需要更新，索引在创建后就不再改变
	class ImmutableIndexTable : public IndexTable
	{
	public:

		//缓冲区的内容必须在创建时就写好，因此需要外部提供一个已经初始化好的常量缓冲
		template<size_t N>
		ImmutableIndexTable(ImmutableCBufferPtr cbufferPtr, const ShaderResourceDesc(&descs)[N]);

		ImmutableIndexTable(const ImmutableIndexTable&) = delete;

		void operator=(const ImmutableIndexTable&) = delete;

		~ImmutableIndexTable();

		Core::D3D12Resource::Buffer* getBuffer() const override;

		D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const override;

		UpdateStruct getUpdateStruct() const override;

		bool updateIndices() override;

	private:

		ImmutableCBufferPtr cbuffer;
	};

	template<size_t N>
	inline ImmutableIndexTable::ImmutableIndexTable(ImmutableCBufferPtr cbufferPtr,
	                                                const ShaderResourceDesc(&descs)[N]) :
		cbuffer(std::move(cbufferPtr))
	{
		if (!cbuffer)
		{
			THROWLOG(LOGERROR() << "不可变类型的索引表必须持有一个常量缓冲！");
		}

		if (cbuffer->getBuffer()->getSize() != byteSize)
		{
			THROWLOG(LOGERROR() << "索引表的常量缓冲大小必须为" << byteSize << "字节！");
		}

		initializeShaderResourceDescs(descs);
	}
}

#endif // !_GEAR_RESOURCE_IMMUTABLEINDEXTABLE_H_

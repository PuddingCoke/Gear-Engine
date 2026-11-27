#pragma once

#ifndef _GEAR_RESOURCE_INDEXTABLE_H_
#define _GEAR_RESOURCE_INDEXTABLE_H_

#include <Gear/Core/D3D12Resource/Buffer.h>

#include <Gear/Core/D3D12Resource/UploadHeap.h>

#include "ResourceBase.h"

namespace Gear::Resource
{
	/// 索引表只能通过根描述符绑定给着色器，无法通过描述符索引访问
	/// 用户只负责更新shaderResourceDescs，资源状态转变与常量更新由引擎在背后完成
	class IndexTable : public ResourceBase
	{
	public:

		//索引表最多能存储的索引数量，恰好填满一个最小的动态常量缓冲子区域
		//网上调研一圈后发现 64 个够用了，光线追踪需要的是SRV Buffer
		static constexpr uint32_t maxIndexNum = 64u;

		//索引表的字节数
		static constexpr uint32_t byteSize = maxIndexNum * 4u;

		IndexTable(const IndexTable&) = delete;

		void operator=(const IndexTable&) = delete;

		virtual ~IndexTable();

		bool getUserUpdate() const;

		//动态类型的索引表没有独立的常量缓冲
		virtual Core::D3D12Resource::Buffer* getBuffer() const = 0;


		virtual D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const = 0;

		struct UpdateStruct
		{
			Core::D3D12Resource::Buffer* const buffer;

			Core::D3D12Resource::UploadHeap* const uploadHeap;

			const uint64_t size;
		};

		//只有updateIndices返回true后才能调用，仅默认类型的索引表有效
		virtual UpdateStruct getUpdateStruct() const = 0;

		//刷新索引，返回缓冲区是否真的需要更新
		//动态类型直接写入DynamicCBuffer的影子缓冲，因此返回值恒为false
		virtual bool updateIndices() = 0;

		const ShaderResourceDesc* getShaderResourceDescsData() const;

		uint64_t getNumShaderResourceDescs() const;

		bool getInUpdateList() const;

		bool getInCheckedList() const;

		void pushToUpdateList();

		void popFromUpdateList();

		void pushToCheckedList();

		void popFromCheckedList();

	protected:

		IndexTable();

		void setUserUpdate(const bool userUpdateParam);

		//记录资源描述，用于资源状态转变
		template<size_t N>
		void initializeShaderResourceDescs(const ShaderResourceDesc(&descs)[N]);

	private:

		std::vector<ShaderResourceDesc> shaderResourceDescs;

		bool userUpdate;

		bool inUpdateList;

		bool inCheckedList;
	};

	template<size_t N>
	inline void IndexTable::initializeShaderResourceDescs(const ShaderResourceDesc(&descs)[N])
	{
		//64个索引恰好对应byteSize字节
		static_assert(N <= maxIndexNum);

		shaderResourceDescs.assign(descs, descs + N);
	}
}

#endif // !_GEAR_RESOURCE_INDEXTABLE_H_

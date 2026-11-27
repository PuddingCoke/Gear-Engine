//#ifndef _GEAR_RESOURCE_INDEXCBUFFER_H_
//#define _GEAR_RESOURCE_INDEXCBUFFER_H_
//
//#include "ResourceBase.h"
//
//#include <Gear/Core/D3D12Resource/UploadHeap.h>
//
//namespace Gear
//{
//	namespace Resource
//	{
//
//		/// Immutable 每帧都不需要更新，但是不能用于索引非持久性资源
//		/// Default 由用户指定是否需要更新，如果索引非持久性资源那么还需要隐式更新
//		/// Dynamic 每帧都需要更新
//		class IndexCBuffer
//		{
//		public:
//
//			bool getUserUpdate() const;
//
//			void setUserUpdate(const bool needUpdateParam);
//
//			bool hasNonPersistentIndex() const;
//
//			D3D12Resource::Buffer* getBuffer() const;
//
//			D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const;
//
//			struct UpdateStruct
//			{
//				D3D12Resource::Buffer* const buffer;
//
//				D3D12Resource::UploadHeap* const uploadHeap;
//
//				const uint64_t size;
//			};
//
//			UpdateStruct getUpdateStruct() const;
//
//			//更新索引，返回是否真的需要更新缓冲区，如果真的需要更新，那么这个方法会一并更新UploadHeap或映射指针指向的内容
//			bool updateIndices();
//
//			bool getInUpdateList() const;
//
//			bool getInCheckedList() const;
//
//			void pushToUpdateList();
//
//			void popFromUpdateList();
//
//			void pushToCheckedList();
//
//			void popFromCheckedList();
//
//			const Resource::ShaderResourceDesc* getShaderResourceDescsData() const;
//
//			uint64_t getNumShaderResourceDescs() const;
//
//		private:
//
//			std::vector<Resource::ShaderResourceDesc> shaderResourceDescs;
//
//			std::vector<uint32_t> resourceIndices;
//
//			D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
//
//			bool needUpdate;
//
//			bool inUpdateList;
//
//			bool inCheckedList;
//		};
//
//	}
//}
//
//#endif // !_GEAR_RESOURCE_INDEXCBUFFER_H_

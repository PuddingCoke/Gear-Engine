#ifndef _GEAR_CORE_SETSHADERCONSTANTSCOMMAND_H_
#define _GEAR_CORE_SETSHADERCONSTANTSCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<D3D12Core::ShaderType ShaderType>
	class SetShaderConstantsCommand :
		public GraphicsCommand,
		public CommandPool<SetShaderConstantsCommand<ShaderType>>
	{
	public:

		void set(const uint64_t dataPtrOffsetParam,
		         const uint32_t numConstantValuesParam,
		         const uint32_t constantWriteOffsetParam)
		{
			dataPtrOffset = dataPtrOffsetParam;

			numConstantValues = numConstantValuesParam;

			constantWriteOffset = constantWriteOffsetParam;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			executer.setShaderConstants<ShaderType>(dataPtrOffset, numConstantValues, constantWriteOffset);
		}

	private:

		/// @brief 数据在大上传缓冲区中的偏移量
		uint64_t dataPtrOffset;

		/// @brief 常量数量
		uint32_t numConstantValues;

		/// @brief 常量写入位置的偏移量
		uint32_t constantWriteOffset;
	};

	using SetVSConstantsCommand = SetShaderConstantsCommand<D3D12Core::ShaderType::VERTEX>;

	using SetHSConstantsCommand = SetShaderConstantsCommand<D3D12Core::ShaderType::HULL>;

	using SetDSConstantsCommand = SetShaderConstantsCommand<D3D12Core::ShaderType::DOMAIN>;

	using SetGSConstantsCommand = SetShaderConstantsCommand<D3D12Core::ShaderType::GEOMETRY>;

	using SetPSConstantsCommand = SetShaderConstantsCommand<D3D12Core::ShaderType::PIXEL>;

	using SetCSConstantsCommand = SetShaderConstantsCommand<D3D12Core::ShaderType::COMPUTE>;

	using SetASConstantsCommand = SetShaderConstantsCommand<D3D12Core::ShaderType::AMPLIFICATION>;

	using SetMSConstantsCommand = SetShaderConstantsCommand<D3D12Core::ShaderType::MESH>;

}

#endif

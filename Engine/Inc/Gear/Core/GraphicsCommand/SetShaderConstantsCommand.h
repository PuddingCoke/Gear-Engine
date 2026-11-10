#ifndef _GEAR_CORE_SETSHADERCONSTANTSCOMMAND_H_
#define _GEAR_CORE_SETSHADERCONSTANTSCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<D3D12Core::ShaderType ShaderType>
	class SetShaderConstantsCommand : public GraphicsCommand
	{
	public:

		SetShaderConstantsCommand() = delete;

		SetShaderConstantsCommand(const uint64_t dataPtrOffset,
		                          const uint32_t numConstantValues,
		                          const uint32_t constantWriteOffset);

		void set(const uint64_t dataPtrOffset,
		         const uint32_t numConstantValues,
		         const uint32_t constantWriteOffset);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		/// @brief 数据在上传缓冲区中的偏移量
		uint64_t dataPtrOffset;

		/// @brief 常量数量
		uint32_t numConstantValues;

		/// @brief 常量写入位置的偏移量
		uint32_t constantWriteOffset;
	};

	template<D3D12Core::ShaderType ShaderType>
	SetShaderConstantsCommand<ShaderType>::SetShaderConstantsCommand(const uint64_t dataPtrOffset,
	                                                                 const uint32_t numConstantValues,
	                                                                 const uint32_t constantWriteOffset)
	{
		set(dataPtrOffset, numConstantValues, constantWriteOffset);
	}

	template<D3D12Core::ShaderType ShaderType>
	void SetShaderConstantsCommand<ShaderType>::set(const uint64_t dataPtrOffset,
	                                                const uint32_t numConstantValues,
	                                                const uint32_t constantWriteOffset)
	{
		this->dataPtrOffset = dataPtrOffset;

		this->numConstantValues = numConstantValues;

		this->constantWriteOffset = constantWriteOffset;
	}

	template<D3D12Core::ShaderType ShaderType>
	void SetShaderConstantsCommand<ShaderType>::execute(GraphicsCommandExecuter& executer)
	{
		executer.setShaderConstants<ShaderType>(dataPtrOffset, numConstantValues, constantWriteOffset);
	}

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

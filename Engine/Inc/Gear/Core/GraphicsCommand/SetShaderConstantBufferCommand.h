#ifndef _GEAR_CORE_SETSHADERCONSTANTBUFFERCOMMAND_H_
#define _GEAR_CORE_SETSHADERCONSTANTBUFFERCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<D3D12Core::ShaderType ShaderType>
	class SetShaderConstantBufferCommand :
		public GraphicsCommand,
		public CommandPool<SetShaderConstantBufferCommand<ShaderType>>
	{
	public:

		void set(const Resource::ImmutableCBuffer& immutableCBuffer)
		{
			this->immutableCBuffer = &immutableCBuffer;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			executer.setConstantBuffer<ShaderType>(*immutableCBuffer);
		}

	private:

		const Resource::ImmutableCBuffer* immutableCBuffer;
	};

	using SetAllShaderConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::ALL>;

	using SetVSConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::VERTEX>;

	using SetHSConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::HULL>;

	using SetDSConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::DOMAIN>;

	using SetGSConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::GEOMETRY>;

	using SetPSConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::PIXEL>;

	using SetCSConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::COMPUTE>;

	using SetASConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::AMPLIFICATION>;

	using SetMSConstantBufferCommand = SetShaderConstantBufferCommand<D3D12Core::ShaderType::MESH>;

}

#endif // _GEAR_CORE_SETSHADERCONSTANTBUFFERCOMMAND_H_

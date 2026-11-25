// #ifndef _GEAR_CORE_SETSHADERINDEXTABLECOMMAND_H_
// #define _GEAR_CORE_SETSHADERINDEXTABLECOMMAND_H_

// #include "GraphicsCommand.h"

// namespace Gear
// {
// 	namespace Core
// 	{

// 		template<D3D12Core::ShaderType ShaderType>
// 		class SetShaderIndexTableCommand :
// 			public GraphicsCommand,
// 			public CommandPool<SetShaderIndexTableCommand<ShaderType>>
// 		{
// 		public:

// 			void set(Resource::IndexTable& indexCBufferParam)
// 			{
// 				indexCBuffer = &indexCBufferParam;
// 			}

// 			void execute(GraphicsCommandExecuter& executer) override
// 			{
// 				executer.setIndexTable<ShaderType>(*indexCBuffer);
// 			}

// 		private:

// 			Resource::IndexTable* indexCBuffer;
// 		};

// 		using SetAllShaderIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::ALL>;

// 		using SetVSIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::VERTEX>;

// 		using SetHSIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::HULL>;

// 		using SetDSIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::DOMAIN>;

// 		using SetGSIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::GEOMETRY>;

// 		using SetPSIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::PIXEL>;

// 		using SetCSIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::COMPUTE>;

// 		using SetASIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::AMPLIFICATION>;

// 		using SetMSIndexTableCommand = SetShaderIndexTableCommand<D3D12Core::ShaderType::MESH>;

// 	}
// }

// #endif // !_GEAR_CORE_SETSHADERINDEXTABLECOMMAND_H_

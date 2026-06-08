#include<Gear/Core/D3D12Core/Shader.h>

#include<Gear/Utils/File.h>

namespace Gear::Core::D3D12Core
{
	ShaderPtr Shader::create(const uint8_t* const bytes, const size_t byteSize)
	{
		return makeUnique<Shader>(bytes, byteSize);
	}

	ShaderPtr Shader::create(const std::wstring& filePath)
	{
		return makeUnique<Shader>(filePath);
	}

	ShaderPtr Shader::create(const std::wstring& filePath, const DXCCompiler::ShaderProfile profile)
	{
		return makeUnique<Shader>(filePath, profile);
	}

	Shader::Shader(const uint8_t* const bytes, const size_t byteSize)
	{
		shaderByteCode.pShaderBytecode = bytes;

		shaderByteCode.BytecodeLength = byteSize;
	}

	Shader::Shader(const std::wstring& filePath)
	{
		if (Utils::File::getExtension(filePath) == L"cso")
		{
			shaderBlob = DXCCompiler::read(filePath);

			shaderByteCode.pShaderBytecode = shaderBlob->GetBufferPointer();

			shaderByteCode.BytecodeLength = shaderBlob->GetBufferSize();

			LOGSUCCESS(L"读取", LogColor::brightBlue, filePath);
		}
		else
		{
			LOGERROR(L"文件的扩展名必须为.cso");
		}
	}

	Shader::Shader(const std::wstring& filePath, const DXCCompiler::ShaderProfile profile)
	{
		if (Utils::File::getExtension(filePath) == L"hlsl")
		{
			shaderBlob = DXCCompiler::compile(filePath, profile);

			shaderByteCode.pShaderBytecode = shaderBlob->GetBufferPointer();

			shaderByteCode.BytecodeLength = shaderBlob->GetBufferSize();

			LOGSUCCESS(L"编译", LogColor::brightBlue, filePath);
		}
		else
		{
			LOGERROR(L"文件的扩展名必须为.hlsl");
		}
	}

	D3D12_SHADER_BYTECODE Shader::getByteCode() const
	{
		return shaderByteCode;
	}
}
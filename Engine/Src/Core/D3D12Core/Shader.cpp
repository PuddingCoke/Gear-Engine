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
		shaderBlob = DXCCompiler::load(bytes, byteSize);

		shaderReflection = DXCCompiler::getReflectionBlob(shaderBlob);
	}

	Shader::Shader(const std::wstring& filePath)
	{
		if (!Utils::File::exist(filePath))
		{
			LOGERROR(filePath, L"指定路径下未找到文件");
		}

		if (Utils::File::getExtension(filePath) == L"cso")
		{
			shaderBlob = DXCCompiler::read(filePath);

			LOGSUCCESS(L"读取", LogColor::brightBlue, filePath);
		}
		else
		{
			LOGERROR(L"文件的扩展名必须为.cso");
		}

		shaderReflection = DXCCompiler::getReflectionBlob(shaderBlob);
	}

	Shader::Shader(const std::wstring& filePath, const DXCCompiler::ShaderProfile profile)
	{
		if (!Utils::File::exist(filePath))
		{
			LOGERROR(filePath, L"指定路径下未找到文件");
		}

		if (Utils::File::getExtension(filePath) == L"hlsl")
		{
			shaderBlob = DXCCompiler::compile(filePath, profile);

			LOGSUCCESS(L"编译", LogColor::brightBlue, filePath);
		}
		else
		{
			LOGERROR(L"文件的扩展名必须为.hlsl");
		}

		shaderReflection = DXCCompiler::getReflectionBlob(shaderBlob);
	}

	ComPtr<ID3D12ShaderReflection> Shader::getReflectionBlob() const
	{
		return shaderReflection;
	}

	D3D12_SHADER_BYTECODE Shader::getByteCode() const
	{
		return D3D12_SHADER_BYTECODE{ shaderBlob->GetBufferPointer(),shaderBlob->GetBufferSize() };
	}
}
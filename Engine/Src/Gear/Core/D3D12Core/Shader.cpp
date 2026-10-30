#include<Gear/Core/D3D12Core/Shader.h>

#include<Gear/Core/D3D12Core/DXCCompiler.h>

namespace Gear::Core::D3D12Core
{
	ShaderPtr Shader::create(const uint8_t* const bytes, const size_t byteSize)
	{
		return makeUnique<Shader>(bytes, byteSize);
	}

	ShaderPtr Shader::create(const Utils::File::CompiledShaderPath& filePath)
	{
		return makeUnique<Shader>(filePath);
	}

	ShaderPtr Shader::create(const Utils::File::ShaderPath& filePath, const ShaderType type)
	{
		return makeUnique<Shader>(filePath, type);
	}

	Shader::Shader(const uint8_t* const bytes, const size_t byteSize)
	{
		shaderBlob = DXCCompiler::load(bytes, byteSize);

		shaderReflection = DXCCompiler::getReflectionBlob(shaderBlob);
	}

	Shader::Shader(const Utils::File::CompiledShaderPath& filePath)
	{
		if (!filePath.exist())
		{
			THROWLOG(LOGERROR() << filePath << "指定路径下未找到文件");
		}

		if (filePath.extension() == L"cso")
		{
			shaderBlob = DXCCompiler::read(filePath);

			LOGSUCCESS() << "读取" << filePath;
		}
		else
		{
			THROWLOG(LOGERROR() << "文件的扩展名必须为.cso");
		}

		shaderReflection = DXCCompiler::getReflectionBlob(shaderBlob);
	}

	Shader::Shader(const Utils::File::ShaderPath& filePath, const ShaderType type)
	{
		if (!filePath.exist())
		{
			THROWLOG(LOGERROR() << filePath << "指定路径下未找到文件");
		}

		if (filePath.extension() == L"hlsl")
		{
			shaderBlob = DXCCompiler::compile(filePath, type);

			LOGSUCCESS() << "编译" << filePath;
		}
		else
		{
			THROWLOG(LOGERROR() << "文件的扩展名必须为.hlsl");
		}

		shaderReflection = DXCCompiler::getReflectionBlob(shaderBlob);
	}

	ComPtr<ID3D12ShaderReflection> Shader::getReflection() const
	{
		return shaderReflection;
	}

	D3D12_SHADER_BYTECODE Shader::getByteCode() const
	{
		return D3D12_SHADER_BYTECODE{ shaderBlob->GetBufferPointer(),shaderBlob->GetBufferSize() };
	}
}

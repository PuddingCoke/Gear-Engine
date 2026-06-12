#pragma once

#ifndef _GEAR_CORE_D3D12CORE_SHADER_H_
#define _GEAR_CORE_D3D12CORE_SHADER_H_

#include<Gear/Core/GraphicsDevice.h>

#include<Gear/Core/D3D12Core/DXCCompiler.h>

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(Shader);

	class Shader
	{
	public:

		Shader() = delete;

		Shader(const Shader&) = delete;

		void operator=(const Shader&) = delete;

		D3D12_SHADER_BYTECODE getByteCode() const;

		static ShaderPtr create(const uint8_t* const bytes, const size_t byteSize);

		static ShaderPtr create(const std::wstring& filePath);

		static ShaderPtr create(const std::wstring& filePath, const DXCCompiler::ShaderProfile profile);

		//raw bytes
		Shader(const uint8_t* const bytes, const size_t byteSize);

		//cso
		Shader(const std::wstring& filePath);

		//hlsl
		Shader(const std::wstring& filePath, const DXCCompiler::ShaderProfile profile);

		ComPtr<ID3D12ShaderReflection> getReflectionBlob() const;

	private:

		D3D12_SHADER_BYTECODE shaderByteCode;

		ComPtr<IDxcBlob> shaderBlob;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_SHADER_H_
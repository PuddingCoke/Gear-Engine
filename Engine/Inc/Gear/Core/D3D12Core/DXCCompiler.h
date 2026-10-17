#pragma once

#ifndef _GEAR_CORE_D3D12CORE_DXCCOMPILER_H_
#define _GEAR_CORE_D3D12CORE_DXCCOMPILER_H_

#include<Gear/Core/GraphicsDevice.h>

#include<dxccompiler/dxcapi.h>

#include<directx/d3d12shader.h>

#include"ShaderType.h"

namespace Gear::Core::D3D12Core::DXCCompiler
{

	//bytes
	ComPtr<IDxcBlob> load(const uint8_t* const bytes, const size_t byteSize);

	//hlsl
	ComPtr<IDxcBlob> compile(const std::wstring& filePath, const ShaderType type);

	//cso
	ComPtr<IDxcBlob> read(const std::wstring& filePath);

	ComPtr<ID3D12ShaderReflection> getReflectionBlob(const ComPtr<IDxcBlob>& shaderBlob);

}

#endif // !_GEAR_CORE_D3D12CORE_DXCCOMPILER_H_

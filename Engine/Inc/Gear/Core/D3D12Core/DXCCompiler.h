#pragma once

#ifndef _GEAR_CORE_D3D12CORE_DXCCOMPILER_H_
#define _GEAR_CORE_D3D12CORE_DXCCOMPILER_H_

#include<Gear/Core/GraphicsDevice.h>

#include<dxccompiler/dxcapi.h>

namespace Gear::Core::D3D12Core::DXCCompiler
{

	enum class ShaderProfile
	{
		VERTEX,
		HULL,
		DOMAIN,
		GEOMETRY,
		PIXEL,
		AMPLIFICATION,
		MESH,
		COMPUTE,
		LIBRARY
	};

	//hlsl
	ComPtr<IDxcBlob> compile(const std::wstring& filePath, const ShaderProfile profile);

	//cso
	ComPtr<IDxcBlob> read(const std::wstring& filePath);

}

#endif // !_GEAR_CORE_D3D12CORE_DXCCOMPILER_H_

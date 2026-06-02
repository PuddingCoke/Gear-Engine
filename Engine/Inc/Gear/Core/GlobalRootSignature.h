#pragma once

#ifndef _GEAR_CORE_GLOBALROOTSIGNATURE_H_
#define _GEAR_CORE_GLOBALROOTSIGNATURE_H_

#include<Gear/Core/D3D12Core/RootSignature.h>

namespace Gear::Core::GlobalRootSignature
{
	//用于使用基础着色器的时候
	const D3D12Core::RootSignature* getBasicShaderRootSignature();

	//用于开启镶嵌细分的时候
	const D3D12Core::RootSignature* getTessellationRootSignature();

	//用于使用几何着色器的时候
	const D3D12Core::RootSignature* getGeometryShaderRootSignature();

	//用于同时使用镶嵌细分和几何着色器的时候
	const D3D12Core::RootSignature* getAllGraphicsShaderRootSignature();

	//用于使用计算着色器的时候
	const D3D12Core::RootSignature* getComputeShaderRootSignature();
}

#endif // !_GEAR_CORE_GLOBALROOTSIGNATURE_H_
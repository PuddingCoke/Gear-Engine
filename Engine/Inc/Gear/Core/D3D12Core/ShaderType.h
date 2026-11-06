#pragma once

#ifndef _GEAR_CORE_D3D12CORE_SHADERTYPE_H_
#define _GEAR_CORE_D3D12CORE_SHADERTYPE_H_

#ifdef VERTEX
#undef VERTEX
#endif // VERTEX

#ifdef HULL
#undef HULL
#endif // HULL

#ifdef DOMAIN
#undef DOMAIN
#endif // DOMAIN

#ifdef GEOMETRY
#undef GEOMETRY
#endif // GEOMETRY

#ifdef PIXEL
#undef PIXEL
#endif // PIXEL

#ifdef COMPUTE
#undef COMPUTE
#endif // COMPUTE

#ifdef AMPLIFICATION
#undef AMPLIFICATION
#endif // AMPLIFICATION

#ifdef MESH
#undef MESH
#endif // MESH

#ifdef TYPECOUNT
#undef TYPECOUNT
#endif // TYPECOUNT

#ifdef LIBRARY
#undef LIBRARY
#endif // LIBRARY

namespace Gear::Core::D3D12Core
{
	enum class ShaderType
	{
		VERTEX, //顶点
		HULL, //外壳
		DOMAIN, //域
		GEOMETRY, //几何
		PIXEL, //像素
		COMPUTE, //计算
		AMPLIFICATION, //放大
		MESH, //网格
		TYPECOUNT, //不包含库类型，因为它没有入口点
		LIBRARY //库
	};
}

#endif // !_GEAR_CORE_D3D12CORE_SHADERTYPE_H_

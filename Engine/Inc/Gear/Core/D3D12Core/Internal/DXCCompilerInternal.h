#pragma once

#ifndef _GEAR_CORE_D3D12CORE_DXCCOMPILER_INTERNAL_H_
#define _GEAR_CORE_D3D12CORE_DXCCOMPILER_INTERNAL_H_

namespace Gear::Core::D3D12Core::DXCCompiler::Internal
{
	void initialize();

	void release();

	//Token（入场券、令牌）
	struct InitializeToken {  InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

}

#endif // !_GEAR_CORE_D3D12CORE_DXCCOMPILER_INTERNAL_H_

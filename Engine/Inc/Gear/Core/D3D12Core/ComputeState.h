#pragma once

#ifndef _GEAR_CORE_D3D12CORE_COMPUTESTATE_H_
#define _GEAR_CORE_D3D12CORE_COMPUTESTATE_H_

#include"Shader.h"

#include"PipelineState.h"

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(ComputeState);

	class ComputeState :public PipelineState
	{
	public:

		ComputeState(ShaderPtr computeShader, const ComPtr<ID3D12PipelineState>& pipelineState, const RootSignature* const rootSignature, const PipelineStateData pipelineStateData);

	private:

		ShaderPtr computeShader;

		ComPtr<ID3D12PipelineState> pipelineState;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_COMPUTESTATE_H_

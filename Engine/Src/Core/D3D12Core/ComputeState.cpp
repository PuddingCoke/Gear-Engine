#include<Gear/Core/D3D12Core/ComputeState.h>

namespace Gear::Core::D3D12Core
{
	ComputeState::ComputeState(ShaderPtr computeShader, const ComPtr<ID3D12PipelineState>& pipelineState, const RootSignature* const rootSignature, const PipelineStateType pipelineStateType, const PipelineStateData pipelineStateData) :
		PipelineState(pipelineState, rootSignature, pipelineStateType, pipelineStateData),
		computeShader(std::move(computeShader))
	{
	}
}

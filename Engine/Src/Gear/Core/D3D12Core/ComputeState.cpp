#include<Gear/Core/D3D12Core/ComputeState.h>

namespace Gear::Core::D3D12Core
{
	ComputeState::ComputeState(ShaderPtr computeShader, const ComPtr<ID3D12PipelineState>& pipelineState, const RootSignature* const rootSignature, const PipelineStateData pipelineStateData) :
		PipelineState(rootSignature, pipelineStateData),
		computeShader(std::move(computeShader)), pipelineState(pipelineState)
	{
		currentPipelineState = pipelineState.Get();
	}
}

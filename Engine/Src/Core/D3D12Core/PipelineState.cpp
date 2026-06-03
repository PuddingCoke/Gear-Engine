#include<Gear/Core/D3D12Core/PipelineState.h>

namespace Gear::Core::D3D12Core
{
	PipelineState::PipelineState(const ComPtr<ID3D12PipelineState>& pipelineState, const RootSignature* const rootSignature, const PipelineStateType pipelineStateType) :
		pipelineState(pipelineState), rootSignature(rootSignature), pipelineStateType(pipelineStateType)
	{
	}

	PipelineState::~PipelineState()
	{
	}

	const RootSignature* PipelineState::getRootSignature() const
	{
		return rootSignature;
	}

	ID3D12PipelineState* PipelineState::get() const
	{
		return pipelineState.Get();
	}

	PipelineState::PipelineStateType PipelineState::getPipelineStateType() const
	{
		return pipelineStateType;
	}
}

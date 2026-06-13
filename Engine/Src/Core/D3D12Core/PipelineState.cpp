#include<Gear/Core/D3D12Core/PipelineState.h>

namespace Gear::Core::D3D12Core
{
	PipelineState::PipelineState(const ComPtr<ID3D12PipelineState>& pipelineState, const RootSignature* const rootSignature, const PipelineStateType pipelineStateType, const PipelineStateData pipelineStateData) :
		pipelineState(pipelineState), rootSignature(rootSignature), pipelineStateType(pipelineStateType), pipelineStateData(pipelineStateData)
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

	PipelineState::PipelineStateData PipelineState::getPipelineStateData() const
	{
		return pipelineStateData;
	}
}

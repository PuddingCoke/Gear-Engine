#include<Gear/Core/D3D12Core/PipelineState.h>

namespace Gear::Core::D3D12Core
{
	PipelineState::PipelineState(const RootSignature* const rootSignature, const PipelineStateData pipelineStateData) :
		currentPipelineState(nullptr), rootSignature(rootSignature), pipelineStateData(pipelineStateData)
	{
	}

	PipelineState::~PipelineState()
	{
	}

	const RootSignature* PipelineState::getRootSignature() const
	{
		return rootSignature;
	}

	ID3D12PipelineState* PipelineState::getPipelineState() const
	{
		return currentPipelineState;
	}

	PipelineState::PipelineStateData PipelineState::getPipelineStateData() const
	{
		return pipelineStateData;
	}
}

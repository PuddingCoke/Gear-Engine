#pragma once

#ifndef _GEAR_CORE_D3D12CORE_PIPELINESTATE_H_
#define _GEAR_CORE_D3D12CORE_PIPELINESTATE_H_

#include"RootSignature.h"

namespace Gear::Core::D3D12Core
{
	class PipelineState
	{
	public:

		enum class PipelineStateType
		{
			GRAPHICS,
			COMPUTE
		};

		PipelineState(const ComPtr<ID3D12PipelineState>& pipelineState, const RootSignature* const rootSignature, const PipelineStateType pipelineStateType);

		~PipelineState();

		const RootSignature* getRootSignature() const;

		ID3D12PipelineState* get() const;

		PipelineStateType getPipelineStateType() const;

	private:

		ComPtr<ID3D12PipelineState> pipelineState;

		//引用
		const RootSignature* const rootSignature;

		const PipelineStateType pipelineStateType;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_PIPELINESTATE_H_

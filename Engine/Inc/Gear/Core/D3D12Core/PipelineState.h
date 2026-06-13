#pragma once

#ifndef _GEAR_CORE_D3D12CORE_PIPELINESTATE_H_
#define _GEAR_CORE_D3D12CORE_PIPELINESTATE_H_

#include"RootSignature.h"

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(PipelineState);

	class PipelineState
	{
	public:

		enum class PipelineStateType
		{
			GRAPHICS,
			COMPUTE
		};

		union PipelineStateData
		{
			struct ComputeData
			{
				DirectX::XMUINT3 groupDimension;
			} computeData;

			struct GraphicsData
			{

			}graphicsData;
		};

		PipelineState(const ComPtr<ID3D12PipelineState>& pipelineState, const RootSignature* const rootSignature, const PipelineStateType pipelineStateType, const PipelineStateData pipelineStateData);

		~PipelineState();

		const RootSignature* getRootSignature() const;

		ID3D12PipelineState* get() const;

		PipelineStateType getPipelineStateType() const;

		PipelineStateData getPipelineStateData() const;

	private:

		ComPtr<ID3D12PipelineState> pipelineState;

		//引用
		const RootSignature* const rootSignature;

		const PipelineStateType pipelineStateType;

		const PipelineStateData pipelineStateData;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_PIPELINESTATE_H_

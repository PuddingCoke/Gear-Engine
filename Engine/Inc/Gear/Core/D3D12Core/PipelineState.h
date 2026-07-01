#pragma once

#ifndef _GEAR_CORE_D3D12CORE_PIPELINESTATE_H_
#define _GEAR_CORE_D3D12CORE_PIPELINESTATE_H_

#include"RootSignature.h"

namespace Gear::Core::D3D12Core
{
	class PipelineState
	{
	public:

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

		PipelineState(const RootSignature* const rootSignature, const PipelineStateData pipelineStateData);

		virtual ~PipelineState();

		const RootSignature* getRootSignature() const;

		ID3D12PipelineState* getPipelineState() const;

		PipelineStateData getPipelineStateData() const;

	protected:

		//引用
		ID3D12PipelineState* currentPipelineState;

	private:

		//引用
		const RootSignature* const rootSignature;

		const PipelineStateData pipelineStateData;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_PIPELINESTATE_H_

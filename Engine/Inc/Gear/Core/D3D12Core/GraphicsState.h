#pragma once

#ifndef _GEAR_CORE_D3D12CORE_GRAPHICSSTATE_H_
#define _GEAR_CORE_D3D12CORE_GRAPHICSSTATE_H_

#include"PipelineState.h"

#include<unordered_map>

namespace Gear::Core::D3D12Core
{
	CREATESAFETYPE(GraphicsState);

	class GraphicsState :public PipelineState
	{
	public:

		GraphicsState(std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements, std::vector<std::string> semanticNames, D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc, const RootSignature* const rootSignature, const PipelineStateData pipelineStateData);

		void updatePipelineState(const DXGI_FORMAT* const rtvFormats, const uint32_t numRenderTargets, const DXGI_FORMAT dsvFormat, const D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);

	private:

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;

		std::vector<std::string> semanticNames;

		//暂存描述结构体，用于在运行时创建图形管线状态
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc;

		uint64_t getUID() const;

		uint64_t currentUID;

		std::unordered_map<uint64_t, ComPtr<ID3D12PipelineState>> pipelineStates;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_GRAPHICSSTATE_H_
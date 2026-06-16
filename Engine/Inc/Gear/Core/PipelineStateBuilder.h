#pragma once

#ifndef _GEAR_CORE_PIPELINESTATEBUILDER_H_
#define _GEAR_CORE_PIPELINESTATEBUILDER_H_

#include<Gear/Core/D3D12Core/PipelineState.h>

#include<Gear/Core/D3D12Core/Shader.h>

#include"FMT.h"

#include"TOPOLOGY.h"

namespace Gear::Core
{
	class PipelineStateBuilder
	{
	public:

		PipelineStateBuilder();

		~PipelineStateBuilder();

		PipelineStateBuilder& setVS(const D3D12Core::Shader& vs);

		PipelineStateBuilder& setHS(const D3D12Core::Shader& hs);

		PipelineStateBuilder& setDS(const D3D12Core::Shader& ds);

		PipelineStateBuilder& setGS(const D3D12Core::Shader& gs);

		PipelineStateBuilder& setPS(const D3D12Core::Shader& ps);

		template<size_t N>
		PipelineStateBuilder& setRTVFormats(const DXGI_FORMAT(&rtvFormats)[N]);

		PipelineStateBuilder& setRTVFormats();

		PipelineStateBuilder& setDSVFormat(const DXGI_FORMAT format);

		template<size_t N>
		PipelineStateBuilder& setInputElements(const D3D12_INPUT_ELEMENT_DESC(&descs)[N]);

		PipelineStateBuilder& setPrimitiveTopologyType(const D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType);

		PipelineStateBuilder& setBlendState(const D3D12_BLEND_DESC& desc);

		PipelineStateBuilder& setRasterizerState(const D3D12_RASTERIZER_DESC& desc);

		PipelineStateBuilder& setDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& desc);

		PipelineStateBuilder& setDefaultFullScreenState();

		D3D12Core::PipelineStatePtr build();

		//用于计算管线状态
		static D3D12Core::PipelineStatePtr build(const D3D12Core::Shader& cs);

	private:

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc;

	};

	template<size_t N>
	inline PipelineStateBuilder& PipelineStateBuilder::setRTVFormats(const DXGI_FORMAT(&rtvFormats)[N])
	{
		memcpy(graphicsDesc.RTVFormats, rtvFormats, sizeof(DXGI_FORMAT) * N);

		graphicsDesc.NumRenderTargets = N;

		return *this;
	}

	template<size_t N>
	inline PipelineStateBuilder& PipelineStateBuilder::setInputElements(const D3D12_INPUT_ELEMENT_DESC(&descs)[N])
	{
		inputElements = std::vector<D3D12_INPUT_ELEMENT_DESC>(descs, descs + N);

		graphicsDesc.InputLayout = { inputElements.data(),static_cast<uint32_t>(inputElements.size()) };

		return *this;
	}
}

#endif // !_GEAR_CORE_PIPELINESTATEBUILDER_H_

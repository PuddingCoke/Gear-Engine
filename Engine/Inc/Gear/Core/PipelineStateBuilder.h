#pragma once

#ifndef _GEAR_CORE_PIPELINESTATEBUILDER_H_
#define _GEAR_CORE_PIPELINESTATEBUILDER_H_

#include<Gear/Core/D3D12Core/GraphicsState.h>

#include<Gear/Core/D3D12Core/ComputeState.h>

#include<Gear/Core/D3D12Core/Shader.h>

namespace Gear::Core
{
	enum class InputClass
	{
		PERVERTEX = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		PERINSTANCE = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA
	};

	constexpr uint32_t ALIGNEDAUTO = D3D12_APPEND_ALIGNED_ELEMENT;

	struct InputElementDesc
	{
		constexpr explicit InputElementDesc(const uint32_t inputSlot, const InputClass inputSlotClass = InputClass::PERVERTEX, const uint32_t instanceDataStepRate = 0, const uint32_t alignedByteOffset = ALIGNEDAUTO) :
			inputSlot(inputSlot), inputSlotClass(inputSlotClass), instanceDataStepRate(instanceDataStepRate), alignedByteOffset(alignedByteOffset)
		{
		}

		uint32_t inputSlot;

		InputClass inputSlotClass;

		uint32_t instanceDataStepRate;

		uint32_t alignedByteOffset;
	};

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
		PipelineStateBuilder& setInputElements(const InputElementDesc(&descs)[N]);

		PipelineStateBuilder& setBlendState(const D3D12_BLEND_DESC& desc);

		PipelineStateBuilder& setRasterizerState(const D3D12_RASTERIZER_DESC& desc);

		PipelineStateBuilder& setDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& desc);

		PipelineStateBuilder& setDefaultFullScreenState();

		D3D12Core::GraphicsStatePtr build();

		//用于计算管线状态
		static D3D12Core::ComputeStatePtr build(D3D12Core::ShaderPtr cs);

	private:

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;

		std::vector<std::string> semanticNames;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc;

	};

	template<size_t N>
	inline PipelineStateBuilder& PipelineStateBuilder::setInputElements(const InputElementDesc(&descs)[N])
	{
		//如果是空的意味着没有调用setVS，那么需要先进行resize
		if (inputElements.empty())
		{
			inputElements.resize(N);
		}
		else if (inputElements.size() != N)
		{
			LOGERROR(L"侦测到", TOWSTRING(descs), L"与", TOWSTRING(inputElements.size()), L"数量不匹配");
		}

		for (uint32_t i = 0; i < N; i++)
		{
			inputElements[i].InputSlot = descs[i].inputSlot;

			inputElements[i].AlignedByteOffset = descs[i].alignedByteOffset;

			inputElements[i].InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>(descs[i].inputSlotClass);

			inputElements[i].InstanceDataStepRate = descs[i].instanceDataStepRate;
		}

		return *this;
	}
}

#endif // !_GEAR_CORE_PIPELINESTATEBUILDER_H_

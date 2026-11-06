#include <Gear/Core/D3D12Core/GraphicsState.h>

#include <Gear/Core/TOPOLOGY.h>

#include <Gear/Core/FMT.h>

namespace Gear::Core::D3D12Core
{
	GraphicsState::GraphicsState(std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements,
	                             std::vector<std::string> semanticNames,
	                             const D3D12_GRAPHICS_PIPELINE_STATE_DESC& graphicsDesc,
	                             const RootSignature* const rootSignature,
	                             const PipelineStateData pipelineStateData) :
		PipelineState(rootSignature, pipelineStateData),
		inputElements(std::move(inputElements)),
		semanticNames(std::move(semanticNames)),
		graphicsDesc(graphicsDesc),
		currentUID(UINT64_MAX)
	{
	}

	void GraphicsState::updatePipelineState(const DXGI_FORMAT* const rtvFormats,
	                                        const uint32_t numRenderTargets,
	                                        const DXGI_FORMAT dsvFormat,
	                                        const D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
	{
		if (numRenderTargets)
		{
			memcpy(graphicsDesc.RTVFormats, rtvFormats, numRenderTargets * sizeof(DXGI_FORMAT));
		}

		graphicsDesc.NumRenderTargets = numRenderTargets;

		graphicsDesc.DSVFormat = dsvFormat;

		graphicsDesc.PrimitiveTopologyType = topologyType;

		const uint64_t uid = getUID();

		if (currentUID != uid)
		{
			const auto it = pipelineStates.find(uid);

			if (it != pipelineStates.cend())
			{
				currentPipelineState = it->second.Get();
			}
			else
			{
				ComPtr<ID3D12PipelineState> pipelineState;

				CHECKERROR(GraphicsDevice::get()->CreateGraphicsPipelineState(&graphicsDesc,
				                                                              IID_PPV_ARGS(&pipelineState)));

				pipelineStates[uid] = pipelineState;

				currentPipelineState = pipelineState.Get();
			}

			currentUID = uid;
		}
	}

	constexpr uint32_t bitsInTotal = sizeof(uint64_t) * 8u;

	constexpr uint32_t primitiveTopologyTypeBits = 2u;

	//000000
	//543210
	//大于越界
#define OFFSETCHECKOUTOFBOUNDS() \
	if (offsetBits > bitsInTotal) \
	{ \
		THROWLOG(LOGERROR() << "侦测到" << TOSTRING(offsetBits) << "越界"); \
	}

	//前几天在研究unordered_map，看了boost那个魔法方法后感觉闹袋有点疼
	//不过我仔细想了想，其实hash和高中学的双射有点联系
	//于是突发奇想想到了这个方法，目前性能还行但是功能还不太完善，等后续有时间了再想想该怎么优化
	uint64_t GraphicsState::getUID() const
	{
		uint64_t uid = 0ull;

		uint32_t offsetBits = 0u;

		for (uint32_t i = 0; i < graphicsDesc.NumRenderTargets; i++)
		{
			const uint64_t formatIndex =
				static_cast<uint64_t>(FMT::getRTVFormatIndex(graphicsDesc.RTVFormats[i]));

			uid |= (formatIndex << offsetBits);

			offsetBits += FMT::getRTVFormatBits();

#ifdef _DEBUG
			OFFSETCHECKOUTOFBOUNDS();
#endif // _DEBUG
		}

		//填充0
		offsetBits +=
			(static_cast<uint32_t>(D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT) - graphicsDesc.NumRenderTargets) *
			FMT::getRTVFormatBits();

		uid |= (static_cast<uint64_t>(FMT::getDSVFormatIndex(graphicsDesc.DSVFormat)) << offsetBits);

		offsetBits += FMT::getDSVFormatBits();

#ifdef _DEBUG
		OFFSETCHECKOUTOFBOUNDS();
#endif // _DEBUG

		if (static_cast<uint64_t>(graphicsDesc.PrimitiveTopologyType) == 0ull)
		{
			THROWLOG(LOGERROR() << "图元拓扑类型不得为" << TOSTRING(TOPOLOGY::TYPE::UNDEFINED));
		}

		//这里减一可以省一个比特位
		//2比特位可以表示POINT LINE TRIANGLE PATCH类型
		uid |= ((static_cast<uint64_t>(graphicsDesc.PrimitiveTopologyType) - 1ull) << offsetBits);

		offsetBits += primitiveTopologyTypeBits;

#ifdef _DEBUG
		OFFSETCHECKOUTOFBOUNDS();
#endif // _DEBUG

		return uid;
	}
}

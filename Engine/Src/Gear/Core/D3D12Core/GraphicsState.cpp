#include<Gear/Core/D3D12Core/GraphicsState.h>

#include<Gear/Utils/Logger.h>

#include<Gear/Core/TOPOLOGY.h>

#include<array>

static constexpr uint32_t rtvFormatToIndex(const DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return 1u;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return 2u;
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 3u;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return 4u;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return 5u;
	case DXGI_FORMAT_R16G16B16A16_UINT:
		return 6u;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return 7u;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return 8u;
	case DXGI_FORMAT_R32G32_FLOAT:
		return 9u;
	case DXGI_FORMAT_R32G32_UINT:
		return 10u;
	case DXGI_FORMAT_R32G32_SINT:
		return 11u;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return 12u;
	case DXGI_FORMAT_R10G10B10A2_UINT:
		return 13u;
	case DXGI_FORMAT_R11G11B10_FLOAT:
		return 14u;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return 15u;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return 16u;
	case DXGI_FORMAT_R8G8B8A8_UINT:
		return 17u;
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return 18u;
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return 19u;
	case DXGI_FORMAT_R16G16_FLOAT:
		return 20u;
	case DXGI_FORMAT_R16G16_UNORM:
		return 21u;
	case DXGI_FORMAT_R16G16_UINT:
		return 22u;
	case DXGI_FORMAT_R16G16_SNORM:
		return 23u;
	case DXGI_FORMAT_R16G16_SINT:
		return 24u;
	case DXGI_FORMAT_R32_FLOAT:
		return 25u;
	case DXGI_FORMAT_R32_UINT:
		return 26u;
	case DXGI_FORMAT_R32_SINT:
		return 27u;
	case DXGI_FORMAT_R8G8_UNORM:
		return 28u;
	case DXGI_FORMAT_R8G8_UINT:
		return 29u;
	case DXGI_FORMAT_R8G8_SNORM:
		return 30u;
	case DXGI_FORMAT_R8G8_SINT:
		return 31u;
	case DXGI_FORMAT_R16_FLOAT:
		return 32u;
	case DXGI_FORMAT_R16_UNORM:
		return 33u;
	case DXGI_FORMAT_R16_UINT:
		return 34u;
	case DXGI_FORMAT_R16_SNORM:
		return 35u;
	case DXGI_FORMAT_R16_SINT:
		return 36u;
	case DXGI_FORMAT_R8_UNORM:
		return 37u;
	case DXGI_FORMAT_R8_UINT:
		return 38u;
	case DXGI_FORMAT_R8_SNORM:
		return 39u;
	case DXGI_FORMAT_R8_SINT:
		return 40u;
	case DXGI_FORMAT_A8_UNORM:
		return 41u;
	case DXGI_FORMAT_B5G6R5_UNORM:
		return 42u;
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 43u;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return 44u;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return 45u;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return 46u;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 47u;
	case DXGI_FORMAT_AYUV:
		return 48u;
	case DXGI_FORMAT_NV12:
		return 49u;
	case DXGI_FORMAT_P010:
		return 50u;
	default:
		return 0u;
	}
}

static constexpr uint32_t dsvFormatToIndex(const DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_D32_FLOAT:
		return 1u;
	case DXGI_FORMAT_D16_UNORM:
		return 2u;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return 3u;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return 4u;
	default:
		return 0u;
	}
}

static constexpr uint64_t formatIndexArrayLength = 256;

static constexpr std::array<uint8_t, formatIndexArrayLength> rtvFormatIndices = []() constexpr
	{
		std::array<uint8_t, formatIndexArrayLength> arr;

		for (uint64_t i = 0; i < formatIndexArrayLength; i++)
		{
			arr[i] = rtvFormatToIndex(static_cast<DXGI_FORMAT>(i));
		}

		return arr;
	}();

static constexpr std::array<uint8_t, formatIndexArrayLength> dsvFormatIndices = []() constexpr
	{
		std::array<uint8_t, formatIndexArrayLength> arr;

		for (uint64_t i = 0; i < formatIndexArrayLength; i++)
		{
			arr[i] = dsvFormatToIndex(static_cast<DXGI_FORMAT>(i));
		}

		return arr;
	}();

namespace Gear::Core::D3D12Core
{
	GraphicsState::GraphicsState(std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements, std::vector<std::string> semanticNames, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& graphicsDesc, const RootSignature* const rootSignature, const PipelineStateData pipelineStateData) :
		PipelineState(rootSignature, pipelineStateData), inputElements(std::move(inputElements)), semanticNames(std::move(semanticNames)), graphicsDesc(graphicsDesc), currentUID(UINT64_MAX)
	{
	}

	void GraphicsState::updatePipelineState(const DXGI_FORMAT* const rtvFormats, const uint32_t numRenderTargets, const DXGI_FORMAT dsvFormat, const D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
	{
		if (numRenderTargets)
		{
			memcpy(graphicsDesc.RTVFormats, rtvFormats, numRenderTargets * sizeof(DXGI_FORMAT));
		}

		graphicsDesc.NumRenderTargets = numRenderTargets;

		graphicsDesc.DSVFormat = dsvFormat;

		graphicsDesc.PrimitiveTopologyType = topologyType;

		const uint64_t uid = getUID();

		if (uid != currentUID)
		{
			const auto it = pipelineStates.find(uid);

			if (it != pipelineStates.cend())
			{
				currentPipelineState = it->second.Get();
			}
			else
			{
				ComPtr<ID3D12PipelineState> pipelineState;

				CHECKERROR(GraphicsDevice::get()->CreateGraphicsPipelineState(&graphicsDesc, IID_PPV_ARGS(&pipelineState)));

				pipelineStates[uid] = pipelineState;

				currentPipelineState = pipelineState.Get();
			}

			currentUID = uid;
		}
	}

	//前几天在研究unordered_map，看了boost那个魔法方法后感觉闹袋有点疼
	//不过我仔细想了想，其实hash和高中学的双射有点联系
	//于是突发奇想想到了这个方法，目前性能还行但是功能还不太完善，等后续有时间了再想想该怎么优化
	uint64_t GraphicsState::getUID() const
	{
		uint64_t uid = 0ull;

		for (uint32_t i = 0; i < graphicsDesc.NumRenderTargets; i++)
		{
			const uint64_t formatIndex = static_cast<uint64_t>(rtvFormatIndices[graphicsDesc.RTVFormats[i]]);

			//6比特位
			uid |= (formatIndex << (i * 6u));
		}

		//3比特位
		uid |= (static_cast<uint64_t>(dsvFormatIndices[graphicsDesc.DSVFormat]) << 48);

		if (static_cast<uint64_t>(graphicsDesc.PrimitiveTopologyType) == 0ull)
		{
			LOGERROR("图元拓扑类型不得为", TOSTRING(TOPOLOGY::TYPE::UNDEFINED));
		}

		//2比特位
		//这里减一可以省一个比特位
		uid |= ((static_cast<uint64_t>(graphicsDesc.PrimitiveTopologyType) - 1ull) << 51);

		return uid;
	}
}

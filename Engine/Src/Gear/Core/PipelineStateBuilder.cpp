#include<Gear/Core/PipelineStateBuilder.h>

#include<Gear/Core/PipelineStateHelper.h>

#include<Gear/Core/GlobalRootSignature.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/FMT.h>

namespace Gear::Core
{
	PipelineStateBuilder::PipelineStateBuilder() :
		graphicsDesc{}
	{
	}

	PipelineStateBuilder::~PipelineStateBuilder()
	{
	}

	/*
		D3D_REGISTER_COMPONENT_UNKNOWN	= 0,
		D3D_REGISTER_COMPONENT_UINT32	= 1,
		D3D_REGISTER_COMPONENT_SINT32	= 2,
		D3D_REGISTER_COMPONENT_FLOAT32	= 3,
		D3D_REGISTER_COMPONENT_UINT16	= 4,
		D3D_REGISTER_COMPONENT_SINT16	= 5,
		D3D_REGISTER_COMPONENT_FLOAT16	= 6,
		D3D_REGISTER_COMPONENT_UINT64	= 7,
		D3D_REGISTER_COMPONENT_SINT64	= 8,
		D3D_REGISTER_COMPONENT_FLOAT64	= 9,
	*/
	DXGI_FORMAT toFMT(const D3D_REGISTER_COMPONENT_TYPE componentType, const BYTE mask)
	{
		if (mask == 1)
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_FLOAT16:
				return FMT::R16F;
			case D3D10_REGISTER_COMPONENT_FLOAT32:
				return FMT::R32F;
			case D3D10_REGISTER_COMPONENT_SINT16:
				return FMT::R16SI;
			case D3D10_REGISTER_COMPONENT_SINT32:
				return FMT::R32SI;
			case D3D10_REGISTER_COMPONENT_UINT16:
				return FMT::R16UI;
			case D3D10_REGISTER_COMPONENT_UINT32:
				return FMT::R32UI;
			default:
				break;
			}
		}

		if (mask <= 3)
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_FLOAT16:
				return FMT::RG16F;
			case D3D10_REGISTER_COMPONENT_FLOAT32:
				return FMT::RG32F;
			case D3D10_REGISTER_COMPONENT_SINT16:
				return FMT::RG16SI;
			case D3D10_REGISTER_COMPONENT_SINT32:
				return FMT::RG32SI;
			case D3D10_REGISTER_COMPONENT_UINT16:
				return FMT::RG16UI;
			case D3D10_REGISTER_COMPONENT_UINT32:
				return FMT::RG32UI;
			default:
				break;
			}
		}

		if (mask <= 7)
		{
			switch (componentType)
			{
			case D3D10_REGISTER_COMPONENT_FLOAT32:
				return FMT::RGB32F;
			case D3D10_REGISTER_COMPONENT_SINT32:
				return FMT::RGB32SI;
			case D3D10_REGISTER_COMPONENT_UINT32:
				return FMT::RGB32UI;
			default:
				break;
			}
		}

		if (mask <= 15)
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_FLOAT16:
				return FMT::RGBA16F;
			case D3D10_REGISTER_COMPONENT_FLOAT32:
				return FMT::RGBA32F;
			case D3D10_REGISTER_COMPONENT_SINT16:
				return FMT::RGBA16SI;
			case D3D10_REGISTER_COMPONENT_SINT32:
				return FMT::RGBA32SI;
			case D3D10_REGISTER_COMPONENT_UINT16:
				return FMT::RGBA16UI;
			case D3D10_REGISTER_COMPONENT_UINT32:
				return FMT::RGBA32UI;
			default:
				break;
			}
		}

		LOGERROR(L"无法得到", static_cast<uint32_t>(componentType), L"和", static_cast<uint32_t>(mask), L"表示的", TOWSTRING(DXGI_FORMAT));

		return FMT::UNKNOWN;
	}

	PipelineStateBuilder& PipelineStateBuilder::setVS(const D3D12Core::Shader& vs)
	{
		graphicsDesc.VS = vs.getByteCode();

		const ComPtr<ID3D12ShaderReflection> reflection = vs.getReflection();

		D3D12_SHADER_DESC shaderDesc = {};

		reflection->GetDesc(&shaderDesc);

		uint32_t numInputElements = shaderDesc.InputParameters;

		for (uint32_t i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC signatureDesc = {};

			reflection->GetInputParameterDesc(i, &signatureDesc);

			const std::string sematicName = signatureDesc.SemanticName;

			if (sematicName == "SV_VERTEXID" || sematicName == "SV_INSTANCEID")
			{
				numInputElements--;
			}
		}

		if (numInputElements == 0u)
		{
			return *this;
		}

		const bool notInitialize = inputElements.empty();

		//如果是空的，意味着这之前没有调用setInputElements，那么需要先进行resize
		if (notInitialize)
		{
			inputElements.resize(numInputElements);
		}
		else if (inputElements.size() != numInputElements)
		{
			LOGERROR(L"侦测到", TOWSTRING(numInputElements), L"与", TOWSTRING(inputElements.size()), L"数量不匹配");
		}

		semanticNames.resize(numInputElements);

		for (uint32_t i = 0; i < numInputElements; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC signatureDesc = {};

			reflection->GetInputParameterDesc(i, &signatureDesc);

			semanticNames[i] = signatureDesc.SemanticName;

			//这里目前还无法完全自动化，取决于具体的实现，用户要决定输入槽位、输入槽位类型、实例数据步近率、对齐字节偏移
			//等引擎后续完善了应该可以免去这四个参数的配置
			D3D12_INPUT_ELEMENT_DESC& inputElement = inputElements[i];
			inputElement.SemanticName = semanticNames[i].c_str();
			inputElement.SemanticIndex = signatureDesc.SemanticIndex;
			inputElement.Format = toFMT(signatureDesc.ComponentType, signatureDesc.Mask);

			//如果之前没调用setInputElements，那么初始化为默认值
			if (notInitialize)
			{
				inputElement.InputSlot = 0u;
				inputElement.AlignedByteOffset = ALIGNEDAUTO;
				inputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				inputElement.InstanceDataStepRate = 0u;
			}
		}

		return *this;
	}

	PipelineStateBuilder& PipelineStateBuilder::setHS(const D3D12Core::Shader& hs)
	{
		graphicsDesc.HS = hs.getByteCode();

		return *this;
	}

	PipelineStateBuilder& PipelineStateBuilder::setDS(const D3D12Core::Shader& ds)
	{
		graphicsDesc.DS = ds.getByteCode();

		return *this;
	}

	PipelineStateBuilder& PipelineStateBuilder::setGS(const D3D12Core::Shader& gs)
	{
		graphicsDesc.GS = gs.getByteCode();

		return *this;
	}

	PipelineStateBuilder& PipelineStateBuilder::setPS(const D3D12Core::Shader& ps)
	{
		graphicsDesc.PS = ps.getByteCode();

		return *this;
	}

	PipelineStateBuilder& PipelineStateBuilder::setBlendState(const D3D12_BLEND_DESC& desc)
	{
		graphicsDesc.BlendState = desc;

		return *this;
	}

	PipelineStateBuilder& PipelineStateBuilder::setRasterizerState(const D3D12_RASTERIZER_DESC& desc)
	{
		graphicsDesc.RasterizerState = desc;

		return *this;
	}

	PipelineStateBuilder& PipelineStateBuilder::setDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& desc)
	{
		graphicsDesc.DepthStencilState = desc;

		return *this;
	}

	PipelineStateBuilder& PipelineStateBuilder::setDefaultFullScreenState()
	{
		return setBlendState(PipelineStateHelper::blendReplace)
			.setRasterizerState(PipelineStateHelper::rasterCullNone)
			.setDepthStencilState(PipelineStateHelper::depthCompareNone)
			.setVS(*GlobalShader::getFullScreenVS());
	}

	D3D12Core::GraphicsStatePtr PipelineStateBuilder::build()
	{
		ComPtr<ID3D12PipelineState> id3d12PipelineState;

		const D3D12Core::RootSignature* selectedRootSignature = nullptr;

		D3D12Core::PipelineState::PipelineStateData pipelineStateData = {};

		const bool hasVertexShader = graphicsDesc.VS.BytecodeLength;

		const bool hasHullShader = graphicsDesc.HS.BytecodeLength;

		const bool hasDomainShader = graphicsDesc.DS.BytecodeLength;

		const bool hasGeometryShader = graphicsDesc.GS.BytecodeLength;

		const bool hasPixelShader = graphicsDesc.PS.BytecodeLength;

		if (!hasVertexShader)
		{
			LOGERROR(L"必须设置顶点着色器！");
		}
		else if ((!hasHullShader && hasDomainShader) || (hasHullShader && !hasDomainShader))
		{
			LOGERROR(L"如果需要镶嵌细分，那么外壳着色器和域着色器必须都被设置！");
		}

		if (!hasHullShader && !hasDomainShader && !hasGeometryShader)
		{
			selectedRootSignature = GlobalRootSignature::getBasicShaderRootSignature();
		}
		else if (hasHullShader && hasDomainShader && !hasGeometryShader)
		{
			selectedRootSignature = GlobalRootSignature::getTessellationRootSignature();
		}
		else if (!hasHullShader && !hasDomainShader && hasGeometryShader)
		{
			selectedRootSignature = GlobalRootSignature::getGeometryShaderRootSignature();
		}
		else
		{
			selectedRootSignature = GlobalRootSignature::getAllGraphicsShaderRootSignature();
		}

		graphicsDesc.pRootSignature = selectedRootSignature->get();
		graphicsDesc.SampleMask = UINT_MAX;
		graphicsDesc.SampleDesc.Count = 1;

		if (inputElements.size())
		{
			graphicsDesc.InputLayout = { inputElements.data(),static_cast<uint32_t>(inputElements.size()) };
		}

		return makeUnique<D3D12Core::GraphicsState>(std::move(inputElements), std::move(semanticNames), graphicsDesc, selectedRootSignature, pipelineStateData);
	}

	D3D12Core::ComputeStatePtr PipelineStateBuilder::build(D3D12Core::ShaderPtr cs)
	{
		ComPtr<ID3D12ShaderReflection> reflection = cs->getReflection();

		{
			D3D12_SHADER_DESC shaderDesc = {};

			reflection->GetDesc(&shaderDesc);

			const uint32_t rawType = D3D12_SHVER_GET_TYPE(shaderDesc.Version);

			if (static_cast<D3D12_SHADER_VERSION_TYPE>(rawType) != D3D12_SHVER_COMPUTE_SHADER)
			{
				LOGERROR(L"侦测到非计算着色器传入！是否读取了错误的文件路径或字节码？");
			}
		}

		ComPtr<ID3D12PipelineState> id3d12PipelineState;

		D3D12Core::PipelineState::PipelineStateData pipelineStateData = {};

		const D3D12Core::RootSignature* selectedRootSignature = GlobalRootSignature::getComputeShaderRootSignature();

		D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
		computeDesc.pRootSignature = selectedRootSignature->get();
		computeDesc.CS = cs->getByteCode();

		CHECKERROR(GraphicsDevice::get()->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&id3d12PipelineState)));

		DirectX::XMUINT3 groupDimension = { 0,0,0 };

		reflection->GetThreadGroupSize(&groupDimension.x, &groupDimension.y, &groupDimension.z);

		if (groupDimension.x == 0 || groupDimension.y == 0 || groupDimension.z == 0)
		{
			LOGERROR(L"无法获取计算着色器的线程组维度信息");
		}

		pipelineStateData.computeData.groupDimension = groupDimension;

		return makeUnique<D3D12Core::ComputeState>(std::move(cs), id3d12PipelineState, selectedRootSignature, pipelineStateData);
	}
}

#include<Gear/Core/PipelineStateBuilder.h>

#include<Gear/Core/PipelineStateHelper.h>

#include<Gear/Core/GlobalRootSignature.h>

#include<Gear/Core/GlobalShader.h>

Gear::Core::PipelineStateBuilder::PipelineStateBuilder() :
	vertexShader(nullptr), hullShader(nullptr), geometryShader(nullptr), domainShader(nullptr), pixelShader(nullptr), computeShader(nullptr), graphicsDesc{}
{
}

Gear::Core::PipelineStateBuilder::~PipelineStateBuilder()
{
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setVS(const D3D12Core::Shader* const vs)
{
	vertexShader = vs;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setHS(const D3D12Core::Shader* const hs)
{
	hullShader = hs;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setDS(const D3D12Core::Shader* const ds)
{
	domainShader = ds;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setGS(const D3D12Core::Shader* const gs)
{
	geometryShader = gs;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setPS(const D3D12Core::Shader* const ps)
{
	pixelShader = ps;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setRTVFormats()
{
	graphicsDesc.NumRenderTargets = 0u;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setDSVFormat(const DXGI_FORMAT format)
{
	graphicsDesc.DSVFormat = format;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setPrimitiveTopologyType(const D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType)
{
	graphicsDesc.PrimitiveTopologyType = primitiveTopologyType;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setBlendState(const D3D12_BLEND_DESC& desc)
{
	graphicsDesc.BlendState = desc;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setRasterizerState(const D3D12_RASTERIZER_DESC& desc)
{
	graphicsDesc.RasterizerState = desc;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& desc)
{
	graphicsDesc.DepthStencilState = desc;

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setDefaultFullScreenState()
{
	return setBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT))
		.setRasterizerState(PipelineStateHelper::rasterCullNone)
		.setDepthStencilState(PipelineStateHelper::depthCompareNone)
		.setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.setVS(GlobalShader::getFullScreenVS());
}

Gear::Core::D3D12Core::PipelineState* Gear::Core::PipelineStateBuilder::build()
{
	ComPtr<ID3D12PipelineState> id3d12PipelineState;

	const D3D12Core::RootSignature* selectedRootSignature = nullptr;

	D3D12Core::PipelineState::PipelineStateType pipelineStateType;

	if (computeShader)
	{
		selectedRootSignature = GlobalRootSignature::getComputeShaderRootSignature();

		pipelineStateType = D3D12Core::PipelineState::PipelineStateType::COMPUTE;

		D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
		computeDesc.pRootSignature = selectedRootSignature->get();
		computeDesc.CS = computeShader->getByteCode();

		CHECKERROR(GraphicsDevice::get()->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&id3d12PipelineState)));
	}
	else
	{
		if (vertexShader)
			graphicsDesc.VS = vertexShader->getByteCode();

		if (hullShader)
			graphicsDesc.HS = hullShader->getByteCode();

		if (domainShader)
			graphicsDesc.DS = domainShader->getByteCode();

		if (geometryShader)
			graphicsDesc.GS = geometryShader->getByteCode();

		if (pixelShader)
			graphicsDesc.PS = pixelShader->getByteCode();

		if (vertexShader == nullptr)
		{
			LOGERROR(L"you must set vertex shader");
		}
		else if ((hullShader == nullptr && domainShader != nullptr) || (hullShader != nullptr && domainShader == nullptr))
		{
			LOGERROR(L"to enable tessellation you must set both hull shader and domain shader");
		}

		if (hullShader == nullptr && domainShader == nullptr && geometryShader == nullptr)
		{
			selectedRootSignature = GlobalRootSignature::getBasicShaderRootSignature();
		}
		else if (hullShader != nullptr && domainShader != nullptr && geometryShader == nullptr)
		{
			selectedRootSignature = GlobalRootSignature::getTessellationRootSignature();
		}
		else if (hullShader == nullptr && domainShader == nullptr && geometryShader != nullptr)
		{
			selectedRootSignature = GlobalRootSignature::getGeometryShaderRootSignature();
		}
		else
		{
			selectedRootSignature = GlobalRootSignature::getAllGraphicsShaderRootSignature();
		}

		pipelineStateType = D3D12Core::PipelineState::PipelineStateType::GRAPHICS;

		graphicsDesc.pRootSignature = selectedRootSignature->get();
		graphicsDesc.SampleMask = UINT_MAX;
		graphicsDesc.SampleDesc.Count = 1;

		CHECKERROR(GraphicsDevice::get()->CreateGraphicsPipelineState(&graphicsDesc, IID_PPV_ARGS(&id3d12PipelineState)));
	}

	return new D3D12Core::PipelineState(id3d12PipelineState, selectedRootSignature, pipelineStateType);
}

Gear::Core::D3D12Core::PipelineState* Gear::Core::PipelineStateBuilder::buildComputeState(const D3D12Core::Shader* const shader)
{
	return PipelineStateBuilder().setCS(shader).build();
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setCS(const D3D12Core::Shader* const cs)
{
	computeShader = cs;

	return *this;
}

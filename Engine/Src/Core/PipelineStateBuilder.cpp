#include<Gear/Core/PipelineStateBuilder.h>

#include<Gear/Core/PipelineStateHelper.h>

#include<Gear/Core/GlobalRootSignature.h>

#include<Gear/Core/GlobalShader.h>

Gear::Core::PipelineStateBuilder::PipelineStateBuilder() :
	graphicsDesc{}
{
}

Gear::Core::PipelineStateBuilder::~PipelineStateBuilder()
{
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setVS(const D3D12Core::Shader* const vs)
{
	if (vs == nullptr)
	{
		LOGERROR(L"顶点着色器指针不能是nullptr");
	}

	graphicsDesc.VS = vs->getByteCode();

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setHS(const D3D12Core::Shader* const hs)
{
	if (hs == nullptr)
	{
		LOGERROR(L"外壳着色器指针不能是nullptr");
	}

	graphicsDesc.HS = hs->getByteCode();

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setDS(const D3D12Core::Shader* const ds)
{
	if (ds == nullptr)
	{
		LOGERROR(L"域着色器指针不能是nullptr");
	}

	graphicsDesc.DS = ds->getByteCode();

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setGS(const D3D12Core::Shader* const gs)
{
	if (gs == nullptr)
	{
		LOGERROR(L"几何着色器指针不能是nullptr");
	}

	graphicsDesc.GS = gs->getByteCode();

	return *this;
}

Gear::Core::PipelineStateBuilder& Gear::Core::PipelineStateBuilder::setPS(const D3D12Core::Shader* const ps)
{
	if (ps == nullptr)
	{
		LOGERROR(L"像素着色器指针不能是nullptr");
	}

	graphicsDesc.PS = ps->getByteCode();

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
	return setBlendState(PipelineStateHelper::blendReplace)
		.setRasterizerState(PipelineStateHelper::rasterCullNone)
		.setDepthStencilState(PipelineStateHelper::depthCompareNone)
		.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE)
		.setVS(GlobalShader::getFullScreenVS());
}

Gear::Core::D3D12Core::PipelineState* Gear::Core::PipelineStateBuilder::build()
{
	ComPtr<ID3D12PipelineState> id3d12PipelineState;

	const D3D12Core::RootSignature* selectedRootSignature = nullptr;

	const bool hasVertexShader = graphicsDesc.VS.BytecodeLength;

	const bool hasHullShader = graphicsDesc.HS.BytecodeLength;

	const bool hasDomainShader = graphicsDesc.DS.BytecodeLength;

	const bool hasGeometryShader = graphicsDesc.GS.BytecodeLength;

	const bool hasPixelShader = graphicsDesc.PS.BytecodeLength;

	if (!hasVertexShader)
	{
		LOGERROR(L"必须设置顶点着色器");
	}
	else if ((!hasHullShader && hasDomainShader) || (hasHullShader && !hasDomainShader))
	{
		LOGERROR(L"如果需要镶嵌细分，那么外壳着色器和域着色器必须都被设置");
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

	CHECKERROR(GraphicsDevice::get()->CreateGraphicsPipelineState(&graphicsDesc, IID_PPV_ARGS(&id3d12PipelineState)));

	return new D3D12Core::PipelineState(id3d12PipelineState, selectedRootSignature, D3D12Core::PipelineState::PipelineStateType::GRAPHICS);
}

Gear::Core::D3D12Core::PipelineState* Gear::Core::PipelineStateBuilder::build(const D3D12Core::Shader* const shader)
{
	if (shader == nullptr)
	{
		LOGERROR(L"计算着色器指针不能是nullptr");
	}

	ComPtr<ID3D12PipelineState> id3d12PipelineState;

	const D3D12Core::RootSignature* selectedRootSignature = GlobalRootSignature::getComputeShaderRootSignature();

	D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
	computeDesc.pRootSignature = selectedRootSignature->get();
	computeDesc.CS = shader->getByteCode();

	CHECKERROR(GraphicsDevice::get()->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&id3d12PipelineState)));

	return new D3D12Core::PipelineState(id3d12PipelineState, selectedRootSignature, D3D12Core::PipelineState::PipelineStateType::COMPUTE);
}

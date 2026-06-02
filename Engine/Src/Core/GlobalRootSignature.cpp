#include<Gear/Core/GlobalRootSignature.h>

#include<Gear/Core/Internal/GlobalRootSignatureInternal.h>

namespace
{
	struct GlobalRootSignatureImpl
	{

		//用于使用顶点和像素着色器的时候
		Gear::Core::D3D12Core::RootSignature* basicShaderRootSignature = nullptr;

		//用于开启镶嵌细分的时候
		Gear::Core::D3D12Core::RootSignature* tessellationRootSignature = nullptr;

		//用于使用几何着色器的时候
		Gear::Core::D3D12Core::RootSignature* geometryShaderRootSignature = nullptr;

		//用于同时使用镶嵌细分和几何着色器的时候
		Gear::Core::D3D12Core::RootSignature* allGraphicsShaderRootSignature = nullptr;

		//用于使用计算着色器的时候
		Gear::Core::D3D12Core::RootSignature* computeShaderRootSignature = nullptr;

	}impl;
}

const Gear::Core::D3D12Core::RootSignature* Gear::Core::GlobalRootSignature::getBasicShaderRootSignature()
{
	return impl.basicShaderRootSignature;
}

const Gear::Core::D3D12Core::RootSignature* Gear::Core::GlobalRootSignature::getTessellationRootSignature()
{
	return impl.tessellationRootSignature;
}

const Gear::Core::D3D12Core::RootSignature* Gear::Core::GlobalRootSignature::getGeometryShaderRootSignature()
{
	return impl.geometryShaderRootSignature;
}

const Gear::Core::D3D12Core::RootSignature* Gear::Core::GlobalRootSignature::getAllGraphicsShaderRootSignature()
{
	return impl.allGraphicsShaderRootSignature;
}

const Gear::Core::D3D12Core::RootSignature* Gear::Core::GlobalRootSignature::getComputeShaderRootSignature()
{
	return impl.computeShaderRootSignature;
}

void Gear::Core::GlobalRootSignature::Internal::initialize()
{
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc[7] = {};

	for (uint32_t i = 0; i < 7; i++)
	{
		samplerDesc[i].ShaderRegister = i;
		samplerDesc[i].RegisterSpace = 0;
		samplerDesc[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	{
		samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[0].MinLOD = 0.0;
		samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
	}

	{
		samplerDesc[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[1].MinLOD = 0;
		samplerDesc[1].MaxLOD = D3D12_FLOAT32_MAX;
	}

	{
		samplerDesc[2].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[2].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[2].MinLOD = 0;
		samplerDesc[2].MaxLOD = D3D12_FLOAT32_MAX;
	}

	{
		samplerDesc[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[3].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[3].MinLOD = 0;
		samplerDesc[3].MaxLOD = D3D12_FLOAT32_MAX;
	}

	{
		samplerDesc[4].Filter = D3D12_FILTER_ANISOTROPIC;
		samplerDesc[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[4].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[4].MaxAnisotropy = 16;
		samplerDesc[4].MinLOD = 0.f;
		samplerDesc[4].MaxLOD = D3D12_FLOAT32_MAX;
	}

	{
		samplerDesc[5].Filter = D3D12_FILTER_ANISOTROPIC;
		samplerDesc[5].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[5].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[5].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[5].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[5].MaxAnisotropy = 16;
		samplerDesc[5].MinLOD = 0.f;
		samplerDesc[5].MaxLOD = D3D12_FLOAT32_MAX;
	}

	{
		samplerDesc[6].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		samplerDesc[6].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc[6].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc[6].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc[6].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		samplerDesc[6].ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER;
		samplerDesc[6].MinLOD = 0.f;
		samplerDesc[6].MaxLOD = D3D12_FLOAT32_MAX;
	}

	//总计 48 DWORDS
	impl.basicShaderRootSignature = new D3D12Core::RootSignature(8u, 0u, 0u, 0u, 32u, 0u, samplerDesc, _countof(samplerDesc),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
		D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);

	impl.basicShaderRootSignature->get()->SetName(L"Basic Shader Root Signature");

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"basic shader root signature", LogColor::defaultColor, L"succeeded");

	//总计 56 DWORDS
	impl.tessellationRootSignature = new D3D12Core::RootSignature(4u, 4u, 12u, 0u, 24u, 0u, samplerDesc, _countof(samplerDesc),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
		D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);

	impl.tessellationRootSignature->get()->SetName(L"Tessellation Root Signature");

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"tessellation root signature", LogColor::defaultColor, L"succeeded");

	//总计 54 DWORDS
	impl.geometryShaderRootSignature = new D3D12Core::RootSignature(8u, 0u, 0u, 8u, 28u, 0u, samplerDesc, _countof(samplerDesc),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
		D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);

	impl.geometryShaderRootSignature->get()->SetName(L"Geometry Shader Root Signature");

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"geometry shader root signature", LogColor::defaultColor, L"succeeded");

	//总计 58 DWORDS
	impl.allGraphicsShaderRootSignature = new D3D12Core::RootSignature(4u, 4u, 8u, 4u, 24u, 0u, samplerDesc, _countof(samplerDesc),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
		D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);

	impl.allGraphicsShaderRootSignature->get()->SetName(L"All Graphics Shader Root Signature");

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"all shader root signature", LogColor::defaultColor, L"succeeded");

	//总计 38 DWORDS
	impl.computeShaderRootSignature = new D3D12Core::RootSignature(0u, 0u, 0u, 0u, 0u, 32u, samplerDesc, _countof(samplerDesc),
		D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
		D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);

	impl.computeShaderRootSignature->get()->SetName(L"Compute Shader Root Signature");

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"compute shader root signature", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalRootSignature::Internal::release()
{
	if (impl.basicShaderRootSignature)
	{
		delete impl.basicShaderRootSignature;
	}

	if (impl.tessellationRootSignature)
	{
		delete impl.tessellationRootSignature;
	}

	if (impl.geometryShaderRootSignature)
	{
		delete impl.geometryShaderRootSignature;
	}

	if (impl.allGraphicsShaderRootSignature)
	{
		delete impl.allGraphicsShaderRootSignature;
	}

	if (impl.computeShaderRootSignature)
	{
		delete impl.computeShaderRootSignature;
	}
}

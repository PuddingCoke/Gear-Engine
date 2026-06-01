#include<Gear/Core/D3D12Core/RootSignature.h>

Gear::Core::D3D12Core::RootSignature::RootSignature(const uint32_t numVSConstants, const uint32_t numHSConstants, const uint32_t numDSConstants, const uint32_t numGSConstants, const uint32_t numPSConstants, const uint32_t numCSConstants, CD3DX12_STATIC_SAMPLER_DESC* const samplerDescs, const uint32_t samplerCount, const D3D12_ROOT_SIGNATURE_FLAGS signatureFlags)
{
	//根据全局和局部结构体的大小和传入的参数计算需要多少个根参数
	const uint32_t numRootParameters = static_cast<uint32_t>(sizeof(ShaderRegisterLayout::ShaderGlobalParameterIndices) / sizeof(uint32_t))
		+ (static_cast<bool>(numVSConstants)
			+ static_cast<bool>(numHSConstants)
			+ static_cast<bool>(numDSConstants)
			+ static_cast<bool>(numGSConstants)
			+ static_cast<bool>(numPSConstants)
			+ static_cast<bool>(numCSConstants)) * static_cast<uint32_t>(sizeof(ShaderRegisterLayout::ShaderLocalParameterIndices) / sizeof(uint32_t));

	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;

	rootParameters.resize(numRootParameters);

	uint32_t rootParameterIndex = 0u;

	//全局根参数的初始化顺序禁止改变

	//由引擎决定的所有着色器可见的全局常量缓冲
	rootParameters[rootParameterIndex++].InitAsConstantBufferView(ShaderRegisterLayout::engineGlobalCBufferRegister, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, D3D12_SHADER_VISIBILITY_ALL);

	//由用户决定的所有着色器可见的全局常量缓冲
	rootParameters[rootParameterIndex++].InitAsConstantBufferView(ShaderRegisterLayout::userGlobalCBufferRegister, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, D3D12_SHADER_VISIBILITY_ALL);

	{
		auto getShaderVisibility = [](const ShaderType shaderType)
			{
				D3D12_SHADER_VISIBILITY shaderVisibility;

				switch (shaderType)
				{
				case ShaderType::VERTEX:
					shaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
					break;
				case ShaderType::HULL:
					shaderVisibility = D3D12_SHADER_VISIBILITY_HULL;
					break;
				case ShaderType::DOMAIN:
					shaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
					break;
				case ShaderType::GEOMETRY:
					shaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
					break;
				case ShaderType::PIXEL:
					shaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
					break;
				case ShaderType::COMPUTE:
					shaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
					break;
				default:
					break;
				}

				return shaderVisibility;
			};

		auto setShaderLocalConstantsParameter = [getShaderVisibility](ShaderRegisterLayout::ShaderLocalParameterIndices& localParameterIndices, CD3DX12_ROOT_PARAMETER1* const rootParameters,
			uint32_t& rootParameterIndex, const ShaderType shaderType, const uint32_t numConstants)
			{
				const D3D12_SHADER_VISIBILITY shaderVisibility = getShaderVisibility(shaderType);

				localParameterIndices.perInvokeConstantsParameterIndex = rootParameterIndex;

				rootParameters[rootParameterIndex++].InitAsConstants(numConstants, ShaderRegisterLayout::perInvokeConstantsRegister, 0, shaderVisibility);
			};

		auto setShaderLocalConstantBufferParameter = [getShaderVisibility](ShaderRegisterLayout::ShaderLocalParameterIndices& localParameterIndices, CD3DX12_ROOT_PARAMETER1* const rootParameters,
			uint32_t& rootParameterIndex, const ShaderType shaderType)
			{
				const D3D12_SHADER_VISIBILITY shaderVisibility = getShaderVisibility(shaderType);

				localParameterIndices.perInvokeCBufferParameterIndex = rootParameterIndex;

				rootParameters[rootParameterIndex++].InitAsConstantBufferView(ShaderRegisterLayout::perInvokeCBufferRegister, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, shaderVisibility);
			};

		auto setShaderLocalParameter = [this, setShaderLocalConstantsParameter, setShaderLocalConstantBufferParameter]
		(CD3DX12_ROOT_PARAMETER1* const rootParameters, uint32_t& rootParameterIndex, const ShaderType shaderType, const uint32_t numConstants)
			{
				setShaderLocalConstantsParameter(localParameterIndices[static_cast<uint32_t>(shaderType)], rootParameters, rootParameterIndex, shaderType, numConstants);

				setShaderLocalConstantBufferParameter(localParameterIndices[static_cast<uint32_t>(shaderType)], rootParameters, rootParameterIndex, shaderType);
			};

		if (numVSConstants)
			setShaderLocalParameter(rootParameters.data(), rootParameterIndex, ShaderType::VERTEX, numVSConstants);

		if (numHSConstants)
			setShaderLocalParameter(rootParameters.data(), rootParameterIndex, ShaderType::HULL, numHSConstants);

		if (numDSConstants)
			setShaderLocalParameter(rootParameters.data(), rootParameterIndex, ShaderType::DOMAIN, numDSConstants);

		if (numGSConstants)
			setShaderLocalParameter(rootParameters.data(), rootParameterIndex, ShaderType::GEOMETRY, numGSConstants);

		if (numPSConstants)
			setShaderLocalParameter(rootParameters.data(), rootParameterIndex, ShaderType::PIXEL, numPSConstants);

		if (numCSConstants)
			setShaderLocalParameter(rootParameters.data(), rootParameterIndex, ShaderType::COMPUTE, numCSConstants);
	}

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};

	rootSignatureDesc.Init_1_1(numRootParameters, rootParameters.data(), samplerCount, samplerDescs, signatureFlags);

	ComPtr<ID3DBlob> signature;

	ComPtr<ID3DBlob> error;

	D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);

	CHECKERROR(GraphicsDevice::get()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

Gear::Core::D3D12Core::RootSignature::~RootSignature()
{
}

ID3D12RootSignature* Gear::Core::D3D12Core::RootSignature::get() const
{
	return rootSignature.Get();
}

uint32_t Gear::Core::D3D12Core::RootSignature::getEngineDefinedGlobalConstantBufferParameterIndex()
{
	return ShaderRegisterLayout::globalParameterIndices.engineGlobalCBufferParameterIndex;
}

uint32_t Gear::Core::D3D12Core::RootSignature::getUserDefinedGlobalConstantBufferParameterIndex()
{
	return ShaderRegisterLayout::globalParameterIndices.userGlobalCBufferParameterIndex;
}

uint32_t Gear::Core::D3D12Core::RootSignature::getVSConstantsParameterIndex() const
{
	return getLocalConstantsParameterIndex(ShaderType::VERTEX);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getVSConstantBufferParameterIndex() const
{
	return getLocalConstantBufferParameterIndex(ShaderType::VERTEX);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getHSConstantsParameterIndex() const
{
	return getLocalConstantsParameterIndex(ShaderType::HULL);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getHSConstantBufferParameterIndex() const
{
	return getLocalConstantBufferParameterIndex(ShaderType::HULL);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getDSConstantsParameterIndex() const
{
	return getLocalConstantsParameterIndex(ShaderType::DOMAIN);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getDSConstantBufferParameterIndex() const
{
	return getLocalConstantBufferParameterIndex(ShaderType::DOMAIN);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getGSConstantsParameterIndex() const
{
	return getLocalConstantsParameterIndex(ShaderType::GEOMETRY);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getGSConstantBufferParameterIndex() const
{
	return getLocalConstantBufferParameterIndex(ShaderType::GEOMETRY);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getPSConstantsParameterIndex() const
{
	return getLocalConstantsParameterIndex(ShaderType::PIXEL);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getPSConstantBufferParameterIndex() const
{
	return getLocalConstantBufferParameterIndex(ShaderType::PIXEL);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getCSConstantsParameterIndex() const
{
	return getLocalConstantsParameterIndex(ShaderType::COMPUTE);
}

uint32_t Gear::Core::D3D12Core::RootSignature::getCSConstantBufferParameterIndex() const
{
	return getLocalConstantBufferParameterIndex(ShaderType::COMPUTE);
}

Gear::Core::D3D12Core::ShaderRegisterLayout::ShaderLocalParameterIndices Gear::Core::D3D12Core::RootSignature::getLocalParameterIndices(const ShaderType shaderType) const
{
	const uint32_t index = static_cast<uint32_t>(shaderType);

#ifdef _DEBUG
	if (localParameterIndices[index].perInvokeConstantsParameterIndex == 0)
	{
		std::wstring errorString = L"there is no root parameter for ";

		switch (shaderType)
		{
		case ShaderType::VERTEX:
			errorString += L"vertex";
			break;
		case ShaderType::HULL:
			errorString += L"hull";
			break;
		case ShaderType::DOMAIN:
			errorString += L"domain";
			break;
		case ShaderType::GEOMETRY:
			errorString += L"geometry";
			break;
		case ShaderType::PIXEL:
			errorString += L"pixel";
			break;
		case ShaderType::COMPUTE:
			errorString += L"compute";
			break;
		default:
			break;
		}

		errorString += L" shader";

		LOGERROR(errorString);
	}
#endif // _DEBUG

	return localParameterIndices[index];
}

uint32_t Gear::Core::D3D12Core::RootSignature::getLocalConstantsParameterIndex(const ShaderType shaderType) const
{
	return getLocalParameterIndices(shaderType).perInvokeConstantsParameterIndex;
}

uint32_t Gear::Core::D3D12Core::RootSignature::getLocalConstantBufferParameterIndex(const ShaderType shaderType) const
{
	return getLocalParameterIndices(shaderType).perInvokeCBufferParameterIndex;
}

#include<D3D12Headers/d3d12shader.h>

#include<dxccompiler/dxcapi.h>

#include<wrl/client.h>

#include<cstdint>

#include<vector>

#include<iostream>

#include"HeaderWriter.h"

using Microsoft::WRL::ComPtr;

std::vector<uint8_t> readAllBinary(const std::wstring& filePath)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		std::wcout << "open " << filePath << " failed\n";
	}
	else
	{
		std::wcout << "open " << filePath << " succeeded\n";
	}

	const size_t fileSize = static_cast<size_t>(file.tellg());

	std::vector<uint8_t> bytes = std::vector<uint8_t>(fileSize);

	file.seekg(0);

	file.read(reinterpret_cast<char*>(bytes.data()), fileSize);

	file.close();

	return bytes;
}

int wmain(int argc, const wchar_t* argv[])
{
	std::locale::global(std::locale(".UTF-8"));

	SetConsoleOutputCP(CP_UTF8);

	const std::wstring rootFolder = getParentFolder(argv[0]);

	const std::wstring filePath = rootFolder + L"\\dummyVS.hlsl";

	std::vector<uint8_t> byteCode = readAllBinary(filePath);

	ComPtr<IDxcCompiler3> compiler;

	ComPtr<IDxcUtils> utils;

	ComPtr<IDxcIncludeHandler> includeHanlder;

	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

	utils->CreateDefaultIncludeHandler(&includeHanlder);

	ComPtr<IDxcCompilerArgs> args;

	utils->BuildArguments(filePath.c_str(), L"main", L"vs_6_6", nullptr, 0, nullptr, 0, &args);

	ComPtr<IDxcBlobEncoding> textBlob;

	utils->CreateBlobFromPinned(byteCode.data(), static_cast<uint32_t>(byteCode.size()), DXC_CP_UTF8, &textBlob);

	DxcBuffer source = {};
	source.Ptr = textBlob->GetBufferPointer();
	source.Size = textBlob->GetBufferSize();
	source.Encoding = DXC_CP_UTF8;

	ComPtr<IDxcResult> result;

	compiler->Compile(&source, args->GetArguments(), args->GetCount(), includeHanlder.Get(), IID_PPV_ARGS(&result));

	ComPtr<IDxcBlobUtf8> errors{};

	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

	//检查一下错误
	if (errors && errors->GetStringLength() > 0)
	{
		const LPCSTR errorMessage = errors->GetStringPointer();

		std::cout << errorMessage << "\n";

		std::cin.get();
	}

	ComPtr<IDxcBlob> reflectionBlob;

	if (result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr) != S_OK)
	{
		std::cout << "get reflection output failed\n";

		std::cin.get();
	}

	const DxcBuffer reflectionBuffer = {
		.Ptr = reflectionBlob->GetBufferPointer(),
		.Size = reflectionBlob->GetBufferSize(),
		.Encoding = 0,
	};

	ComPtr<ID3D12ShaderReflection> shaderReflection{};

	if (utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection)) != S_OK)
	{
		std::cout << "create reflection failed\n";

		std::cin.get();
	}

	D3D12_SHADER_BUFFER_DESC shaderBufferDesc{};

	ID3D12ShaderReflectionConstantBuffer* constantBuffer = shaderReflection->GetConstantBufferByIndex(0);

	ID3D12ShaderReflectionVariable* reflectionVariable = constantBuffer->GetVariableByIndex(0);

	ID3D12ShaderReflectionType* structType = reflectionVariable->GetType();

	D3D12_SHADER_TYPE_DESC typeDesc;

	structType->GetDesc(&typeDesc);

	HeaderWriter writer(rootFolder);

	writer.beginDesc(NAMESPACE, L"Gear");

	writer.beginDesc(NAMESPACE, L"Core");

	writer.beginDesc(NAMESPACE, L"D3D12Core");

	writer.beginDesc(STRUCT, L"PerframeResource");

	uint32_t currentWriteSize = 0u;

	for (uint32_t i = 0; i < typeDesc.Members; i++)
	{
		const char* name = structType->GetMemberTypeName(i);

		ID3D12ShaderReflectionType* const memberType = structType->GetMemberTypeByIndex(i);

		D3D12_SHADER_TYPE_DESC memberDesc;

		memberType->GetDesc(&memberDesc);

		//std::cout << memberDesc.Class << " " << memberDesc.Type << " " << memberDesc.Columns * memberDesc.Rows * 4 << " " << name << "\n";

		if (memberDesc.Class == D3D_SHADER_VARIABLE_CLASS::D3D_SVC_SCALAR)
		{
			currentWriteSize += 4u;

			switch (memberDesc.Type)
			{
			case D3D_SVT_UINT:
				writer.writeLine() << L"uint32_t";
				break;
			case D3D_SVT_FLOAT:
				writer.writeLine() << L"float";
				break;
			default:
				break;
			}
		}
		else if (memberDesc.Class == D3D_SHADER_VARIABLE_CLASS::D3D_SVC_VECTOR)
		{
			const uint32_t vectorSize = memberDesc.Columns * memberDesc.Rows * 4;

			currentWriteSize += vectorSize;

			writer.writeLine() << L"DirectX::XM";

			switch (memberDesc.Type)
			{
			case D3D_SVT_UINT:
				writer.write() << L"UINT";
				break;
			case D3D_SVT_FLOAT:
				writer.write() << L"FLOAT";
				break;
			default:
				break;
			}

			if (vectorSize == 8ull)
			{
				writer.write() << L"2";
			}
			else if (vectorSize == 12ull)
			{
				writer.write() << L"3";
			}
			else if (vectorSize == 16ull)
			{
				writer.write() << L"4";
			}
		}
		else if (memberDesc.Class == D3D_SHADER_VARIABLE_CLASS::D3D_SVC_MATRIX_COLUMNS)
		{
			currentWriteSize += 64;

			writer.writeLine() << "DirectX::XMMATRIX";
		}

		writer.write() << L" " << name << L";";
	}

	if (currentWriteSize != 512 && currentWriteSize != 1024)
	{
		uint32_t targetSize = 0u;

		if (currentWriteSize < 512)
		{
			targetSize = 512u;
		}
		else if (currentWriteSize < 1024)
		{
			//1024字节足够满足引擎每帧需要提供的数据量
			targetSize = 1024u;
		}
		else
		{
			//报错
		}

		const uint32_t gap = 16u - currentWriteSize % 16u;

		if (gap == 4u)
		{
			writer.writeLine() << "uint32_t padding0;";

			currentWriteSize += gap;
		}
		else if (gap == 8u)
		{
			writer.writeLine() << L"DirectX::XMUINT2 padding0;";

			currentWriteSize += gap;
		}
		else if (gap == 12u)
		{
			writer.writeLine() << L"DirectX::XMUINT3 padding0;";

			currentWriteSize += gap;
		}

		if (targetSize - currentWriteSize)
		{
			const uint32_t num = (targetSize - currentWriteSize) / 16u;

			writer.writeLine() << "DirectX::XMUINT4 padding1[" << std::to_wstring(num) << "];";
		}
	}


	writer.endDesc();

	writer.endDesc();

	writer.endDesc();

	writer.endDesc();

	writer.close();

	std::cin.get();

	return 0;
}
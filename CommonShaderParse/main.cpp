#include<D3D12Headers/d3d12shader.h>

#include<dxccompiler/dxcapi.h>

#include<wrl/client.h>

#include<vector>

#include<cwctype>

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

//根据分隔符利用find_first_of进行分割
std::vector<std::wstring> split(std::wstring str, const wchar_t separator)
{
	std::vector<std::wstring> result;

	size_t idx = str.find_first_of(separator);

	while (idx != std::wstring::npos)
	{
		const std::wstring splitFront = str.substr(0ull, idx);

		const std::wstring splitBack = str.substr(idx + 1ull, str.length() - idx - 1ull);

		str = splitBack;

		result.emplace_back(splitFront);

		idx = str.find_first_of(separator);
	}

	result.emplace_back(str);

	return result;
}

//进行合并
std::wstring concatenate(const std::vector<std::wstring>& vec)
{
	std::wstring result = L"";

	for (const std::wstring& str : vec)
	{
		result += str;
	}

	return result;
}

std::wstring toUpper(const std::wstring& str)
{
	std::wstring result = L"";

	for (const wchar_t& c : str)
	{
		result.push_back(std::towupper(c));
	}

	return result;
}

std::wstring toLower(const std::wstring& str)
{
	std::wstring result = L"";

	for (const wchar_t& c : str)
	{
		result.push_back(std::towlower(c));
	}

	return result;
}

std::wstring toCamel(const std::wstring& str)
{
	size_t idx = 1ull;

	if (str.length() == 1)
	{
		return toUpper(str);
	}

	if (str == L"CBUFFER")
	{
		idx = 2ull;
	}

	return toUpper(str.substr(0ull, idx)) + toLower(str.substr(idx, str.length() - idx));
}

//关键词检测
bool vectorHas(const std::vector<std::wstring>& vec, const std::wstring& str)
{
	for (const std::wstring& s : vec)
	{
		if (s == str)
		{
			return true;
		}
	}

	return false;
}

//获取寄存器值
uint32_t getRegisterValue(const std::wstring& str)
{
	return static_cast<uint32_t>(std::stoi(str.substr(1ull)));
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

	HeaderWriter writer(rootFolder);

	{
		ID3D12ShaderReflectionConstantBuffer* constantBuffer = shaderReflection->GetConstantBufferByIndex(0);

		ID3D12ShaderReflectionVariable* reflectionVariable = constantBuffer->GetVariableByIndex(0);

		ID3D12ShaderReflectionType* structType = reflectionVariable->GetType();

		D3D12_SHADER_TYPE_DESC typeDesc;

		structType->GetDesc(&typeDesc);

		writer.beginDesc(DescType::NAMESPACE, L"Gear");

		writer.beginDesc(DescType::NAMESPACE, L"Core");

		writer.beginDesc(DescType::NAMESPACE, L"D3D12Core");

		writer.beginDesc(DescType::NAMESPACE, L"CommonShaderLayout");

		writer.beginDesc(DescType::STRUCT, L"PerframeResource");

		uint32_t currentWriteSize = 0u;

		for (uint32_t i = 0; i < typeDesc.Members; i++)
		{
			const char* name = structType->GetMemberTypeName(i);

			ID3D12ShaderReflectionType* const memberType = structType->GetMemberTypeByIndex(i);

			D3D12_SHADER_TYPE_DESC memberDesc;

			memberType->GetDesc(&memberDesc);

			//根据vectorSize和memberDesc.type推导类型
			if (memberDesc.Class == D3D_SHADER_VARIABLE_CLASS::D3D_SVC_SCALAR || memberDesc.Class == D3D_SHADER_VARIABLE_CLASS::D3D_SVC_VECTOR)
			{
				const uint32_t vectorSize = memberDesc.Columns * memberDesc.Rows * 4;

				currentWriteSize += vectorSize;

				uint32_t writeType;

				switch (memberDesc.Type)
				{
				case D3D_SVT_INT:
					writeType = static_cast<uint32_t>(VarType::INT);
					break;
				case D3D_SVT_UINT:
					writeType = static_cast<uint32_t>(VarType::UINT);
					break;
				case D3D_SVT_FLOAT:
					writeType = static_cast<uint32_t>(VarType::FLOAT);
					break;
				default:
					break;
				}

				writeType += vectorSize / 4u - 1u;

				writer.writeVar(static_cast<VarType>(writeType), name);
			}
			//目前好像只用MATRIX，这里先不管了
			else if (memberDesc.Class == D3D_SHADER_VARIABLE_CLASS::D3D_SVC_MATRIX_COLUMNS)
			{
				currentWriteSize += 64;

				writer.writeVar(VarType::MATRIX, name);
			}
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

			//离对齐有距离
			if (currentWriteSize % 16u)
			{
				const uint32_t gap = 16u - currentWriteSize % 16u;

				const uint32_t writeType = static_cast<uint32_t>(VarType::UINT) + gap / 4u - 1u;

				writer.writeVar(static_cast<VarType>(writeType), "padding0");

				currentWriteSize += gap;
			}

			//对齐后可能正好满足，所以要进行检测
			if (targetSize - currentWriteSize)
			{
				const uint32_t num = (targetSize - currentWriteSize) / 16u;

				writer.writeVar(VarType::UINT4, "padding1", num);
			}
		}
	}

	writer.endDescQuick(L"CommonShaderLayout");

	{
		std::wifstream inputStream = std::wifstream(rootFolder + L"\\Common.hlsli");

		//用于全局参数对
		uint32_t rootParameterIndex = 0;

		std::vector<std::pair<std::wstring, uint32_t>> globalParameterPair;

		std::vector<std::pair<std::wstring, uint32_t>> localParameterPair;

		std::vector<std::pair<std::wstring, uint32_t>> registerPair;

		while (true)
		{
			std::wstring str;

			std::getline(inputStream, str);

			//先用空格分割
			const std::vector<std::wstring> splitResult = split(str, L' ');

			if (splitResult.back()[0] != L'b')
			{
				break;
			}

			const std::wstring name = splitResult[1];

			std::vector<std::wstring> subName = split(name, L'_');

			std::vector<std::wstring> lowerCamelSubName = subName;

			lowerCamelSubName[0] = toLower(lowerCamelSubName[0]);

			for (uint32_t i = 1; i < lowerCamelSubName.size(); i++)
			{
				lowerCamelSubName[i] = toCamel(lowerCamelSubName[i]);
			}

			const std::wstring lowerCamelName = concatenate(lowerCamelSubName);

			registerPair.push_back(std::pair<std::wstring, uint32_t>
				(lowerCamelName + L"Register", getRegisterValue(splitResult.back())));

			//通过GLOBAL关键词进行识别
			if (vectorHas(subName, L"GLOBAL"))
			{
				globalParameterPair.push_back(std::pair<std::wstring, uint32_t>
					(lowerCamelName + L"ParameterIndex", rootParameterIndex));
			}
			else
			{
				localParameterPair.push_back(std::pair<std::wstring, uint32_t>
					(lowerCamelName + L"ParameterIndex", 0u));
			}

			rootParameterIndex++;
		}

		{
			writer.beginDesc(VarModifier::CONSTEXPR, DescType::STRUCT, L"ShaderGlobalParameterIndices");

			for (const auto& pair : globalParameterPair)
			{
				writer.writeUint(VarModifier::NONE, pair.first, pair.second);
			}

			writer.endDesc(L"globalParameterIndices");
		}

		{
			writer.beginDesc(VarModifier::NONE, DescType::STRUCT, L"ShaderLocalParameterIndices");

			for (const auto& pair : localParameterPair)
			{
				writer.writeUint(VarModifier::NONE, pair.first, pair.second);
			}

			writer.endDesc();
		}

		{
			for (const auto& pair : registerPair)
			{
				writer.writeUint(VarModifier::CONSTEXPR, pair.first, pair.second);
			}
		}

		/*std::wcout << "global parameter\n";

		for (const auto& a : globalParameterPair)
		{
			std::wcout << a.first << " " << a.second << "\n";
		}

		std::wcout << "local parameter\n";

		for (const auto& a : localParameterPair)
		{
			std::wcout << a.first << " " << 0ull << "\n";
		}

		std::wcout << "register\n";

		for (const auto& a : registerPair)
		{
			std::wcout << a.first << " " << a.second << "\n";
		}*/

		inputStream.close();
	}

	writer.endDescQuick(L"");

	writer.close();

	std::cin.get();

	return 0;
}
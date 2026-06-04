#pragma once

#include<fstream>

#include<string>

#include<stack>

#include<cstdint>

#include<iostream>

enum class DescType;

enum class VarType;

enum class VarModifier;

std::wstring getParentFolder(const std::wstring& filePath)
{
	size_t idx = filePath.find_last_of(L'\\');

	if (idx == std::wstring::npos)
	{
		idx = filePath.find_last_of(L'/');

		if (idx == std::wstring::npos)
		{
			return L"";
		}
	}

	return filePath.substr(0, idx);
}

class HeaderWriter
{
public:

	HeaderWriter(const std::wstring& rootFolder);

	~HeaderWriter();

	void close();

	void foward();

	void backward();

	void beginDesc(const VarModifier modifier, const DescType type, const wchar_t* name);

	void beginDesc(const DescType type, const wchar_t* name);

	//仅用于着色器反射
	void writeVar(const VarType type, const char* const name, const uint32_t numElement = 1u);

	void writeUint(const VarModifier modifier, const std::wstring& name, const uint32_t value);

	//快速退出并跳转到
	void endDescQuick(const std::wstring& name);

	//退出并留下变量名
	void endDesc(const std::wstring& varName = L"");

	std::wostream& writeLine();

	std::wostream& write();

	std::wofstream outStream;

private:

	struct Desc
	{
		//记录描述类型，因为struct等需要在花括号后放置';'
		const DescType descType;

		//记录名称用来快速退出
		const std::wstring name;
	};

	size_t currentCount;

	const std::wstring outDirectory;

	std::stack<Desc> descStack;

};

inline HeaderWriter::HeaderWriter(const std::wstring& rootFolder) :
	currentCount(0), outDirectory(getParentFolder(getParentFolder(getParentFolder(rootFolder))) + L"\\Engine\\Inc\\Gear\\Core\\D3D12Core\\CommonShaderLayout.h")
{
	outStream = std::wofstream(outDirectory);

	writeLine() << L"//由 CommonShaderParse.exe 自动生成，请勿修改";

	writeLine();

	writeLine() << L"#pragma once";

	writeLine();

	writeLine() << L"#include<DirectXMath.h>";

	writeLine();

	writeLine() << L"#include<cstdint>";

	writeLine();
}

HeaderWriter::~HeaderWriter()
{
	if (outStream.is_open())
	{
		outStream.close();
	}
}

inline void HeaderWriter::close()
{
	std::wcout << L"write file to " << outDirectory << L"\n";

	outStream.close();
}

inline void HeaderWriter::foward()
{
	currentCount++;
}

inline void HeaderWriter::backward()
{
	if (currentCount == 0ull)
	{
		std::wcout << L"不能再后退了！\n";

		return;
	}

	currentCount--;
}

#undef CONST

enum class VarModifier
{
	NONE,
	CONST,
	CONSTEXPR
};

enum class DescType
{
	ENUM,
	UNION,
	STRUCT,
	NAMESPACE
};

inline void HeaderWriter::beginDesc(const VarModifier modifier, const DescType type, const wchar_t* name)
{
	descStack.push({ type,std::wstring(name) });

	switch (modifier)
	{
	case VarModifier::NONE:
		writeLine();
		break;
	case VarModifier::CONST:
		writeLine() << L"const ";
		break;
	case VarModifier::CONSTEXPR:
		writeLine() << L"constexpr ";
		break;
	default:
		break;
	}

	switch (type)
	{
	case DescType::ENUM:
		write() << L"enum";
		break;
	case DescType::STRUCT:
		write() << L"struct";
		break;
	case DescType::UNION:
		write() << L"union";
		break;
	case DescType::NAMESPACE:
		write() << L"namespace";
		break;
	default:
		break;
	}

	write() << L" " << name;

	writeLine() << L"{";

	foward();
}

inline void HeaderWriter::beginDesc(const DescType type, const wchar_t* name)
{
	beginDesc(VarModifier::NONE, type, name);
}

enum class VarType
{
	FLOAT,//float
	FLOAT2,//float2
	FLOAT3,//float3
	FLOAT4,//float4
	UINT,//uint
	UINT2,//uint2
	UINT3,//uint3
	UINT4,//uint4
	MATRIX,//matrix
	INT,//int
	INT2,//int2
	INT3,//int3
	INT4//int4
};

inline void HeaderWriter::writeVar(const VarType type, const char* const name, const uint32_t numElement)
{
	switch (type)
	{
	case VarType::FLOAT:
		writeLine() << L"float";
		break;
	case VarType::FLOAT2:
		writeLine() << L"DirectX::XMFLOAT2";
		break;
	case VarType::FLOAT3:
		writeLine() << L"DirectX::XMFLOAT3";
		break;
	case VarType::FLOAT4:
		writeLine() << L"DirectX::XMFLOAT4";
		break;
	case VarType::UINT:
		writeLine() << L"uint32_t";
		break;
	case VarType::UINT2:
		writeLine() << L"DirectX::XMUINT2";
		break;
	case VarType::UINT3:
		writeLine() << L"DirectX::XMUINT3";
		break;
	case VarType::UINT4:
		writeLine() << L"DirectX::XMUINT4";
		break;
	case VarType::MATRIX:
		writeLine() << L"DirectX::XMMATRIX";
		break;
	case VarType::INT:
		writeLine() << L"int32_t";
		break;
	case VarType::INT2:
		writeLine() << L"DirectX::XMINT2";
		break;
	case VarType::INT3:
		writeLine() << L"DirectX::XMINT3";
		break;
	case VarType::INT4:
		writeLine() << L"DirectX::XMINT4";
		break;
	default:
		break;
	}

	write() << L" " << name;

	if (numElement > 1)
	{
		write() << L"[" << numElement << L"]";
	}

	write() << L";";
}

inline void HeaderWriter::writeUint(const VarModifier modifier, const std::wstring& name, const uint32_t value)
{
	switch (modifier)
	{
	case VarModifier::NONE:
		writeLine();
		break;
	case VarModifier::CONST:
		writeLine() << L"const ";
		break;
	case VarModifier::CONSTEXPR:
		writeLine() << L"constexpr ";
		break;
	default:
		break;
	}

	write() << L"uint32_t" << L" " << name << L" = " << value << L";";
}

inline void HeaderWriter::endDescQuick(const std::wstring& name)
{
	while (!descStack.empty() && descStack.top().name != name)
	{
		endDesc();
	}
}

inline void HeaderWriter::endDesc(const std::wstring& varName)
{
	backward();

	writeLine() << L"}" << varName;

	const DescType type = descStack.top().descType;

	if (type == DescType::STRUCT || type == DescType::UNION || type == DescType::ENUM)
	{
		write() << L";\n";
	}

	descStack.pop();
}

inline std::wostream& HeaderWriter::writeLine()
{
	return write() << L"\n" << std::wstring(currentCount, L'\t');
}

inline std::wostream& HeaderWriter::write()
{
	return outStream;
}

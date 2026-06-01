#pragma once

#include<fstream>

#include<string>

#include<stack>

enum DescType
{
	ENUM,
	UNION,
	STRUCT,
	NAMESPACE
};

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

	void beginDesc(const DescType type, const wchar_t* name);

	void endDesc();

	std::wostream& writeLine();

	std::wostream& write();

	std::wofstream outStream;

private:

	size_t currentCount;

	const std::wstring solutionDirectory;

	std::stack<DescType> descStack;

};

inline HeaderWriter::HeaderWriter(const std::wstring& rootFolder) :
	currentCount(0), solutionDirectory(getParentFolder(getParentFolder(getParentFolder(rootFolder))))
{
	outStream = std::wofstream(solutionDirectory + L"\\Engine\\Inc\\Gear\\Core\\D3D12Core\\CommonShaderLayout.h");

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
}

inline void HeaderWriter::close()
{
	std::wcout << L"write file to " << solutionDirectory + L"\\Engine\\Inc\\Gear\\Core\\D3D12Core\\CommonShaderLayout.h\n";

	outStream.close();
}

inline void HeaderWriter::foward()
{
	currentCount++;
}

inline void HeaderWriter::backward()
{
	currentCount--;
}

inline void HeaderWriter::beginDesc(const DescType type, const wchar_t* name)
{
	descStack.push(type);

	switch (type)
	{
	case DescType::ENUM:
		writeLine() << L"enum";
		break;
	case DescType::STRUCT:
		writeLine() << L"struct";
		break;
	case DescType::UNION:
		writeLine() << L"union";
		break;
	case DescType::NAMESPACE:
		writeLine() << L"namespace";
		break;
	default:
		break;
	}

	write() << L" " << name;

	writeLine() << L"{";

	foward();
}

inline void HeaderWriter::endDesc()
{
	backward();

	writeLine() << L"}";

	if (descStack.top() == STRUCT || descStack.top() == UNION || descStack.top() == ENUM)
	{
		write() << L";";
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

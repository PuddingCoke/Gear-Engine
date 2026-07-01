#include<Gear/Utils/File.h>

#include<Gear/Utils/Internal/FileInternal.h>

#include<Gear/Utils/Logger.h>

#include<fstream>

#include<sstream>

#include<algorithm>

#include<vector>

#include<Windows.h>

namespace Gear::Utils::File
{
	namespace Internal
	{
		struct FileImpl
		{
			std::wstring rootFolder;
		}impl;

		void setRootFolder(const std::wstring& rootFolder)
		{
			impl.rootFolder = rootFolder;
		}
	}

	std::wstring getRootFolder()
	{
		return Internal::impl.rootFolder;
	}

	std::wstring backslashToSlash(const std::wstring& filePath)
	{
		std::wstring result = filePath;

		std::replace(result.begin(), result.end(), L'\\', L'/');

		return result;
	}

	std::wstring slashToBackSlash(const std::wstring& filePath)
	{
		std::wstring result = filePath;

		std::replace(result.begin(), result.end(), L'/', L'\\');

		return result;
	}

	std::wstring getParentFolder(const std::wstring& filePath)
	{
		if (filePath.empty())
		{
			return filePath;
		}

		std::wstring trueFilePath = filePath;

		if (filePath[filePath.length() - 1] == L'\\' || filePath[filePath.length() - 1] == L'/')
		{
			trueFilePath = trueFilePath.substr(0, filePath.length() - 1ull);
		}

		size_t idx = trueFilePath.find_last_of(L'\\');

		if (idx != std::wstring::npos)
		{
			return trueFilePath.substr(0, idx) + L"\\";
		}
		else
		{
			idx = trueFilePath.find_last_of(L'/');

			if (idx != std::wstring::npos)
			{
				return trueFilePath.substr(0, idx) + L"/";
			}
		}

		return L"";
	}

	std::wstring getExtension(const std::wstring& filePath)
	{
		const size_t idx = filePath.find_last_of(L'.');

		if (idx == std::wstring::npos)
		{
			return L"";
		}

		return filePath.substr(idx + 1, filePath.size() - idx - 1);
	}

	std::wstring readAllText(const std::wstring& filePath)
	{
		std::wifstream file(filePath);

		if (!file.is_open())
		{
			LOGERROR(L"打开文件", filePath, L"失败！");
		}

		std::wstringstream stringStream;

		stringStream << file.rdbuf();

		return stringStream.str();
	}

	std::vector<uint8_t> readAllBinary(const std::wstring& filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			LOGERROR(L"打开文件", filePath, L"失败！");
		}

		const size_t fileSize = static_cast<size_t>(file.tellg());

		std::vector<uint8_t> bytes = std::vector<uint8_t>(fileSize);

		file.seekg(0);

		file.read(reinterpret_cast<char*>(bytes.data()), fileSize);

		file.close();

		return bytes;
	}

	bool exist(const std::wstring& filePath)
	{
		return GetFileAttributesW(filePath.c_str()) != INVALID_FILE_ATTRIBUTES;
	}
}
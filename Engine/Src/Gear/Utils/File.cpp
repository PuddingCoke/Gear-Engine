#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include<Gear/Utils/File.h>

#include<Gear/Utils/Internal/FileInternal.h>

#include<Gear/Utils/Log.h>

#include<Gear/Utils/String.h>

#include<fstream>

#include<sstream>

#include<algorithm>

#include<vector>

#include<Windows.h>

#include<filesystem>

namespace Gear::Utils::File
{

	Path::Path(const std::wstring& pathStr)
	{
		if (pathStr == L"")
		{
			THROWLOG(LOGERROR() << "路径为空！");
		}

		this->pathStr = std::filesystem::path(pathStr).has_root_path() ? pathStr : (getAssetsFolder() + pathStr);
	}

	Path::Path(const wchar_t* const pathStr)
	{
		if (pathStr == nullptr)
		{
			THROWLOG(LOGERROR() << "路径指针为nullptr！");
		}

		if (pathStr[0] == '\0')
		{
			THROWLOG(LOGERROR() << "路径为空！");
		}

		this->pathStr = std::filesystem::path(pathStr).has_root_path() ? pathStr : (getAssetsFolder() + pathStr);
	}

	const std::wstring& Path::str() const
	{
		return pathStr;
	}

	const wchar_t* Path::cstr() const
	{
		return pathStr.c_str();
	}

	Path Path::operator+(const std::wstring& subdirectory)
	{
		return Path(pathStr + subdirectory);
	}

	Path& Path::operator+=(const std::wstring& subdirectory)
	{
		pathStr += subdirectory;

		return *this;
	}

	bool Path::exist() const
	{
		return GetFileAttributesW(pathStr.c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	std::wstring Path::extension() const
	{
		return File::getExtension(pathStr);
	}

	std::string Path::multibyteStr() const
	{
		return String::wStringToString(pathStr);
	}

	EngineAssetsPath::EngineAssetsPath(const std::wstring& subdirectory) :
		Path(std::wstring(L"EngineAssets/") + subdirectory)
	{
	}

	EngineAssetsPath::EngineAssetsPath(const wchar_t* const subdirectory) :
		Path(std::wstring(L"EngineAssets/") + subdirectory)
	{
	}

	RootPath::RootPath(const std::wstring& subdirectory) :
		Path(getRootFolder() + subdirectory)
	{
	}

	RootPath::RootPath(const wchar_t* const subdirectory) :
		Path(getRootFolder() + subdirectory)
	{
	}

	UserDataPath::UserDataPath(const std::wstring& subdirectory) :
		Path(getUserDataFolder() + subdirectory)
	{
	}

	UserDataPath::UserDataPath(const wchar_t* const subdirectory) :
		Path(getUserDataFolder() + subdirectory)
	{
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

	std::wstring readAllText(const Path& filePath)
	{
		std::wifstream file(filePath.str());

		if (!file.is_open())
		{
			THROWLOG(LOGERROR() << "打开文件" << filePath << "失败！");
		}

		std::wstringstream stringStream;

		stringStream << file.rdbuf();

		return stringStream.str();
	}

	std::vector<uint8_t> readAllBinary(const Path& filePath)
	{
		std::ifstream file(filePath.str(), std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			THROWLOG(LOGERROR() << "打开文件" << filePath << "失败！");
		}

		const size_t fileSize = static_cast<size_t>(file.tellg());

		std::vector<uint8_t> bytes = std::vector<uint8_t>(fileSize);

		file.seekg(0);

		file.read(reinterpret_cast<char*>(bytes.data()), fileSize);

		file.close();

		return bytes;
	}

	struct FileImpl
	{

		FileImpl(const std::wstring& wRootFolder);

		const std::wstring rootFolder;

		const std::wstring assetsFolder;

		const std::wstring userDataFolder;

	};

	FileImpl::FileImpl(const std::wstring& rootFolder) :
		rootFolder(rootFolder),
		assetsFolder(rootFolder + L"Assets/"),
		userDataFolder(rootFolder + L"UserData/")
	{
		std::filesystem::create_directories(userDataFolder);
	}

	UniquePtr<FileImpl> impl;

	namespace Internal
	{
		void initialize(const std::wstring& rootFolder)
		{
			impl = makeUnique<FileImpl>(rootFolder);
		}

		void release()
		{
			impl.reset();
		}
	}

	std::wstring getRootFolder()
	{
		return impl->rootFolder;
	}

	std::wstring getAssetsFolder()
	{
		return impl->assetsFolder;
	}

	std::wstring getUserDataFolder()
	{
		return impl->userDataFolder;
	}

}

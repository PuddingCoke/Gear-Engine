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
	std::wstring getExecutablePathW()
	{
		std::wstring buffer(MAX_PATH, L'\0');

		while (true)
		{
			const DWORD len = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

			if (len == 0)
			{
				throw std::runtime_error("无法获取可执行程序文件路径");
			}

			if (len < buffer.size())
			{
				buffer.resize(len);

				break;
			}

			buffer.resize(buffer.size() * 2);
		}

		return buffer;
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

		if (filePath.back() == L'\\' || filePath.back() == L'/')
		{
			trueFilePath = trueFilePath.substr(0, filePath.length() - 1ull);
		}

		size_t idx = trueFilePath.find_last_of(L'\\');

		if (idx != std::wstring::npos)
		{
			return trueFilePath.substr(0, idx + 1ull);
		}
		else
		{
			idx = trueFilePath.find_last_of(L'/');

			if (idx != std::wstring::npos)
			{
				return trueFilePath.substr(0, idx + 1ull);
			}
		}

		return filePath;
	}

	std::wstring getExtension(const std::wstring& filePath)
	{
		const size_t idx = filePath.find_last_of(L'.');

		if (idx == std::wstring::npos)
		{
			return L"";
		}

		return filePath.substr(idx + 1ull, filePath.size() - (idx + 1ull));
	}

	class FileImpl
	{
	public:

		FileImpl();

		const std::wstring& getRootFolder() const;

		const std::wstring& getAssetsFolder() const;

		const std::wstring& getUserDataFolder() const;

	private:

		const std::wstring rootFolder;

		const std::wstring assetsFolder;

		const std::wstring userDataFolder;

	};

	FileImpl::FileImpl() :
		rootFolder(backslashToSlash(getParentFolder(getExecutablePathW()))),
		assetsFolder(rootFolder + L"Assets/"),
		userDataFolder(rootFolder + L"UserData/")
	{
		std::filesystem::create_directories(userDataFolder);
	}

	const std::wstring& FileImpl::getRootFolder() const
	{
		return rootFolder;
	}

	const std::wstring& FileImpl::getAssetsFolder() const
	{
		return assetsFolder;
	}

	const std::wstring& FileImpl::getUserDataFolder() const
	{
		return userDataFolder;
	}

	UniquePtr<FileImpl> impl;

	Path::Path(const std::wstring& pathStr)
	{
		if (pathStr == L"")
		{
			THROWLOG(LOGERROR() << "路径为空！");
		}

		this->pathStr = std::filesystem::path(pathStr).has_root_path() ? pathStr : (impl->getAssetsFolder() + pathStr);
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

		this->pathStr = std::filesystem::path(pathStr).has_root_path() ? pathStr : (impl->getAssetsFolder() + pathStr);
	}

	const std::wstring& Path::str() const
	{
		return pathStr;
	}

	std::string Path::multibyteStr() const
	{
		return String::wStringToString(pathStr);
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

	Path Path::parentPath() const
	{
		return Path(getParentFolder(pathStr));
	}

	bool Path::exist() const
	{
		return GetFileAttributesW(pathStr.c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	std::wstring Path::extension() const
	{
		return File::getExtension(pathStr);
	}

#define GEAR_IMPL_PATH_CLASS(ClassName, PrefixExpr)							\
		ClassName::ClassName(const std::wstring& subdirectory) :            \
			Path((PrefixExpr) + subdirectory)								\
		{																	\
		}																	\
		ClassName::ClassName(const wchar_t* const subdirectory) :           \
			Path((PrefixExpr) + subdirectory)								\
		{																	\
		}																	

	GEAR_IMPL_PATH_CLASS(EngineAssetsPath, std::wstring(L"EngineAssets/"));
	GEAR_IMPL_PATH_CLASS(RootPath, impl->getRootFolder());
	GEAR_IMPL_PATH_CLASS(AssetsPath, impl->getAssetsFolder());
	GEAR_IMPL_PATH_CLASS(UserDataPath, impl->getUserDataFolder());

#undef GEAR_IMPL_PATH_CLASS

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

	namespace Internal
	{
		void initialize()
		{
			impl = makeUnique<FileImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

}

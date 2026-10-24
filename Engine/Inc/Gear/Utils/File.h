#pragma once

#ifndef _GEAR_UTILS_FILE_H_
#define _GEAR_UTILS_FILE_H_

#include<string>

#include<vector>

namespace Gear::Utils::File
{

	//路径
	//相对路径会被重定向到资源目录
	//若要访问其它位置请使用绝对路径
	class Path
	{
	public:

		Path() = delete;

		Path(const std::wstring& pathStr);

		Path(const wchar_t* const pathStr);

		Path(const Path&) = default;

		const std::wstring& str() const;

		const wchar_t* cstr() const;

		//Path + L"ASD/"		Path/ASD/
		//Path + L"ASD.exe"		Path/ASD.exe
		Path operator+(const std::wstring& subdirectory);

		//Path += L"ASD/"		Path/ASD/
		//Path += L"ASD.exe"	Path/ASD.exe
		Path& operator+=(const std::wstring& subdirectory);

		bool exist() const;

		std::wstring extension() const;

		std::string multibyteStr() const;

	private:

		std::wstring pathStr;

	};

	//获取EXE的绝对目录
	std::wstring getRootFolder();

	std::wstring getAssetsFolder();

	std::wstring getUserDataFolder();

	std::wstring backslashToSlash(const std::wstring& filePath);

	std::wstring slashToBackSlash(const std::wstring& filePath);

	std::wstring getParentFolder(const std::wstring& filePath);

	std::wstring getExtension(const std::wstring& filePath);

	std::wstring readAllText(const Path& filePath);

	std::vector<uint8_t> readAllBinary(const Path& filePath);

#define GEAR_MAKE_PATH_CLASS(ClassName, PrefixExpr)              \
    class ClassName : public Path                                \
    {                                                            \
    public:                                                      \
        ClassName() = delete;                                    \
        ClassName(const std::wstring& subdirectory) :            \
            Path((PrefixExpr) + subdirectory)                    \
        {                                                        \
        }                                                        \
        ClassName(const wchar_t* const subdirectory) :           \
            Path((PrefixExpr) + subdirectory)                    \
        {                                                        \
        }                                                        \
    }

	GEAR_MAKE_PATH_CLASS(EngineAssetsPath, std::wstring(L"EngineAssets/"));
	GEAR_MAKE_PATH_CLASS(RootPath, getRootFolder());
	GEAR_MAKE_PATH_CLASS(UserDataPath, getUserDataFolder());

#undef GEAR_MAKE_PATH_CLASS

}

#endif // !_GEAR_UTILS_FILE_H_

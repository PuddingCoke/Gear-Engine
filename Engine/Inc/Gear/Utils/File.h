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

	class EngineAssetsPath :public Path
	{
	public:

		EngineAssetsPath() = delete;

		EngineAssetsPath(const std::wstring& subdirectory);

		EngineAssetsPath(const wchar_t* const subdirectory);

	};

	class RootPath :public Path
	{
	public:

		RootPath() = delete;

		RootPath(const std::wstring& subdirectory);

		RootPath(const wchar_t* const subdirectory);

	};

	class UserDataPath :public Path
	{
	public:

		UserDataPath() = delete;

		UserDataPath(const std::wstring& subdirectory);

		UserDataPath(const wchar_t* const subdirectory);

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

}

#endif // !_GEAR_UTILS_FILE_H_

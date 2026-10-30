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

		std::string multibyteStr() const;

		const wchar_t* cstr() const;

		//Path + L"ASD/"		Path/ASD/
		//Path + L"ASD.exe"		Path/ASD.exe
		Path operator+(const std::wstring& subdirectory);

		//Path += L"ASD/"		Path/ASD/
		//Path += L"ASD.exe"	Path/ASD.exe
		Path& operator+=(const std::wstring& subdirectory);

		Path parentPath() const;

		bool exist() const;

		std::wstring extension() const;

	private:

		std::wstring pathStr;

	};

#define GEAR_DECLARE_PATH_CLASS(ClassName)				\
	class ClassName: public Path						\
	{													\
	public:												\
		ClassName() = delete;							\
		ClassName(const std::wstring& subdirectory);	\
		ClassName(const wchar_t* const subdirectory);	\
	}

	GEAR_DECLARE_PATH_CLASS(EngineAssetsPath);
	GEAR_DECLARE_PATH_CLASS(RootPath);
	GEAR_DECLARE_PATH_CLASS(AssetsPath);
	GEAR_DECLARE_PATH_CLASS(UserDataPath);
	GEAR_DECLARE_PATH_CLASS(ShaderPath);
	GEAR_DECLARE_PATH_CLASS(CompiledShaderPath);

#undef GEAR_DECLARE_PATH_CLASS

	std::wstring readAllText(const Path& filePath);

	std::vector<uint8_t> readAllBinary(const Path& filePath);

}

#endif // !_GEAR_UTILS_FILE_H_

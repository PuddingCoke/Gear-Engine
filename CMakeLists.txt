# =============================================================================
# Gear Engine - 顶层 CMake 构建脚本
#
# 构建方式：Ninja + MSVC（cl.exe），不依赖 Visual Studio IDE。
# Ninja 生成器会输出 compile_commands.json，供 clangd 等工具使用。
#
# 用法（先在同一个 cmd 进程中初始化 MSVC 环境，vcvars 会把 cl.exe 和 ninja.exe
# 加入 PATH）：
#   call "<VS>\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.51
#   cmake -S . -B out/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build out/debug
#
# 构建目录不要用 build/：Windows 文件系统不区分大小写，会和现有 MSBuild
# 输出目录 Build/ 混在一起。这里统一使用 out/ 作为 CMake 的构建目录。
# =============================================================================

cmake_minimum_required(VERSION 3.25)

project(GearEngine LANGUAGES CXX)

# -----------------------------------------------------------------------------
# 平台与工具链
# -----------------------------------------------------------------------------
if(NOT WIN32)
    message(FATAL_ERROR "Gear Engine 仅支持 Windows。")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "Gear Engine 需要 MSVC 工具链。")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Gear Engine 仅支持 x64。")
endif()

# -----------------------------------------------------------------------------
# 语言标准与默认构建类型
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 单配置生成器（Ninja）未指定 CMAKE_BUILD_TYPE 时默认 Debug，
# 否则 $<CONFIG> 为空，输出目录会退化。
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug/Release)" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif()

# Ninja/Makefile 生成器下导出编译数据库
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# 目录约定（与现有 MSBuild 布局保持一致）
# -----------------------------------------------------------------------------
set(GEAR_ROOT_DIR    "${CMAKE_CURRENT_SOURCE_DIR}")
set(GEAR_ENGINE_DIR  "${GEAR_ROOT_DIR}/Engine")
set(GEAR_LIBS_DIR    "${GEAR_ROOT_DIR}/Libs")
set(GEAR_DLLS_DIR    "${GEAR_ROOT_DIR}/Dlls")
set(GEAR_OUTPUT_ROOT "${GEAR_ROOT_DIR}/Build")

foreach(dir "${GEAR_LIBS_DIR}" "${GEAR_DLLS_DIR}")
    if(NOT EXISTS "${dir}")
        message(FATAL_ERROR "缺少依赖目录：${dir}")
    endif()
endforeach()

# -----------------------------------------------------------------------------
# 全局编译选项（与原 vcxproj 对齐）
#   /utf-8      等价于 /source-charset:utf-8 /execution-charset:utf-8
#   /permissive- 对应 ConformanceMode
#   /sdl        对应 SDLCheck
# 警告级别（/W3、/W4）由各 target 自行设置。
# -----------------------------------------------------------------------------
add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:/utf-8>
    $<$<COMPILE_LANGUAGE:CXX>:/permissive->
    $<$<COMPILE_LANGUAGE:CXX>:/sdl>
)

# 字符集：对应各工程 vcxproj 的 <CharacterSet>Unicode</CharacterSet>。
# 这些定义会出现在 %(PreprocessorDefinitions) 的继承值中（_UNICODE UNICODE），
# 缺少时会选中 Win32 API 的 ...A 版本（如 CreateWindowExA）。
add_compile_definitions(UNICODE _UNICODE)

# -----------------------------------------------------------------------------
# 预编译依赖库
# -----------------------------------------------------------------------------
set(GEAR_DXCOMPILER_LIB "${GEAR_LIBS_DIR}/dxcompiler.lib")
if(NOT EXISTS "${GEAR_DXCOMPILER_LIB}")
    message(FATAL_ERROR "缺少依赖库：${GEAR_DXCOMPILER_LIB}")
endif()

# -----------------------------------------------------------------------------
# HLSL 着色器编译（全局统一设置，具体规则见 cmake/GearShader.cmake）
# -----------------------------------------------------------------------------
# dxc.exe 的位置约定：<Windows Kits>/10/bin/<SDK 版本>/x64/dxc.exe
#   - 只看 bin：Redist 下的是运行库副本，不参与编译
#   - 固定 x64：工程只构建 x64
#   - 多个 SDK 共存时取版本号最大的一个（自然排序，避免 10.0.9 排在 10.0.10 之后）
# 也可以用 -DGEAR_DXC_EXECUTABLE=<path> 显式指定。
set(GEAR_DXC_EXECUTABLE "" CACHE FILEPATH "显式指定 dxc.exe；留空则在 Windows Kits 下自动查找")

if(NOT GEAR_DXC_EXECUTABLE)
    # Windows Kits 根目录：优先读注册表（安装路径可能被改过），再退回 ProgramFiles(x86)
    get_filename_component(_gear_kits_root
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
        ABSOLUTE)
    if(NOT _gear_kits_root OR NOT EXISTS "${_gear_kits_root}")
        set(_gear_kits_root "$ENV{ProgramFiles\(x86\)}/Windows Kits/10")
    endif()

    file(GLOB _gear_dxc_candidates "${_gear_kits_root}/bin/10.*/x64/dxc.exe")
    if(_gear_dxc_candidates)
        list(SORT _gear_dxc_candidates COMPARE NATURAL ORDER ASCENDING)
        list(GET _gear_dxc_candidates -1 GEAR_DXC_EXECUTABLE)
    endif()
endif()

if(NOT GEAR_DXC_EXECUTABLE)
    message(FATAL_ERROR
        "未找到 dxc.exe。预期位置：<Windows Kits>/10/bin/<SDK 版本>/x64/dxc.exe\n"
        "请安装 Windows SDK，或通过 -DGEAR_DXC_EXECUTABLE=<path> 显式指定。")
endif()
if(NOT EXISTS "${GEAR_DXC_EXECUTABLE}")
    message(FATAL_ERROR "dxc.exe 不存在：${GEAR_DXC_EXECUTABLE}")
endif()
message(STATUS "着色器编译器 dxc：${GEAR_DXC_EXECUTABLE}")

# 所有项目统一使用同一个着色器模型
set(GEAR_SHADER_MODEL "6.6" CACHE STRING "所有项目统一的 HLSL 着色器模型")

include("${GEAR_ROOT_DIR}/cmake/GearShader.cmake")
include("${GEAR_ROOT_DIR}/cmake/GearDemo.cmake")

# -----------------------------------------------------------------------------
# 编译器能力探测（Release 的 WholeProgramOptimization 需要）
# -----------------------------------------------------------------------------
include(CheckIPOSupported)
check_ipo_supported(RESULT GEAR_IPO_SUPPORTED LANGUAGES CXX)

add_subdirectory(CommonShaderParse)
add_subdirectory(Engine)
add_subdirectory(Demos)

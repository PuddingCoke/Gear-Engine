# =============================================================================
# GearDemo.cmake —— 示例项目（Demos/*、TODOAPP）的公共构建规则
#
# 对应 SampleProject.vcxproj + Demos/DemoCommon.props：
#   IncludePath                  -> Engine/External、Engine/Inc
#   LibraryPath                  -> Libs（全部静态库由 GearStaticLibs 目标统一链接）
#   OutDir                       -> Build/<项目>/<Config>
#   WarningLevel/SDLCheck/字符集 -> 顶层统一（/W3 默认，另有 /sdl /utf-8 /permissive-）
#   PreprocessorDefinitions      -> _CONSOLE + _DEBUG/NDEBUG
#   AdditionalDependencies       -> GearStaticLibs（自动处理 -d 后缀）+ LIBS 追加
#   PostBuildEvent(xcopy)        -> Demo 的 Assets、Runtime/Assets、Runtime/Dlls 三份拷贝
#   FxCompile(ObjectFileOutput)  -> Assets/Shaders/CompiledShaders/<相对路径>.cso
#                                   （转交 GearShader.cmake）
#   DemoCommon.props(/DELAYLOAD) -> 自动收集 Runtime/Dlls/*.dll 生成 /DELAYLOAD
#
# 公共默认用普通变量定义，写在 Demos/CMakeLists.txt 里即可被所有子目录继承：
#   set(GEAR_DEMO_LIBS         ...)   # 额外第三方库（不带 .lib 后缀）
#   set(GEAR_DEMO_INCLUDE_DIRS ...)
#   set(GEAR_DEMO_DEFINITIONS  ...)
#   set(GEAR_DEMO_SHADER_INCLUDE_DIRS ...)
#   set(GEAR_DEMO_SHADER_DEFINITIONS  ...)
#
# 单个 Demo 的 CMakeLists.txt 只需要：
#   gear_add_demo(BlackHole
#       SHADERS
#           "ColorGradingPS.hlsl|Pixel"
#           "BlackHolePS.hlsl|Pixel")
# SOURCES / HEADERS 省略时自动取当前目录的 *.cpp / *.h。
# SHADERS 中的路径相对 <当前目录>/PrecompileShaders。
# =============================================================================

if(NOT DEFINED GEAR_DXC_EXECUTABLE)
    message(FATAL_ERROR "GearDemo.cmake 需要先由顶层 CMakeLists 完成着色器相关初始化")
endif()

# -----------------------------------------------------------------------------
# 内部：扫描 Libs 下所有静态库并按 Debug/Release 变体配对
#
# 约定：Foo.lib 与 Foo-d.lib 是同一个库的两个配置，Debug 用 -d、其余配置用
# 普通版；只有一种变体时直接链接它。Engine 的产物同样输出到 Libs，必须排除，
# 否则会与 Engine 目标重复链接。
#
# 不用 CONFIGURE_DEPENDS：Engine.lib 是构建产物，让它触发重新 configure 会
# 在每次干净构建后多一轮配置。第三方库是静态文件，配置时扫描一次足够。
# -----------------------------------------------------------------------------
function(_gear_scan_static_libs OUT_VAR)
    file(GLOB _libs "${GEAR_LIBS_DIR}/*.lib")

    set(_bases "")
    foreach(_lib IN LISTS _libs)
        # 不能用 NAME_WE：它按“最长扩展名”截断，会把
        # GFSDK_SSAO_D3D12.win64.lib 变成 GFSDK_SSAO_D3D12
        get_filename_component(_name "${_lib}" NAME)
        string(REGEX REPLACE "\\.[Ll][Ii][Bb]$" "" _name "${_name}")
        if(_name MATCHES "^Engine(-d)?$")
            continue()
        endif()
        string(REGEX REPLACE "-d$" "" _base "${_name}")
        list(APPEND _bases "${_base}")
    endforeach()
    list(REMOVE_DUPLICATES _bases)

    set(_resolved "")
    foreach(_base IN LISTS _bases)
        set(_release "${GEAR_LIBS_DIR}/${_base}.lib")
        set(_debug   "${GEAR_LIBS_DIR}/${_base}-d.lib")
        if(EXISTS "${_release}" AND EXISTS "${_debug}")
            list(APPEND _resolved
                "$<$<CONFIG:Debug>:${_debug}>"
                "$<$<NOT:$<CONFIG:Debug>>:${_release}>")
        elseif(EXISTS "${_debug}")
            # 只有 Debug 版：只能在 Debug 下链接
            list(APPEND _resolved "$<$<CONFIG:Debug>:${_debug}>")
        else()
            list(APPEND _resolved "${_release}")
        endif()
    endforeach()

    set(${OUT_VAR} "${_resolved}" PARENT_SCOPE)
endfunction()

_gear_scan_static_libs(_GEAR_STATIC_LIBS)

# 所有第三方静态库的集合。目标（示例项目、工具等）只需链接它，
# 不需要再逐个列举 Libs 下的库名。
if(NOT TARGET GearStaticLibs)
    add_library(GearStaticLibs INTERFACE)
    target_link_libraries(GearStaticLibs INTERFACE ${_GEAR_STATIC_LIBS})
endif()

# -----------------------------------------------------------------------------
# 内部：解析额外库（GEAR_DEMO_LIBS / A_LIBS）
#   绝对路径          -> 原样
#   Libs 下的库名      -> 绝对路径，自动处理 -d 变体
#   其它名字          -> 原样交给链接器（系统库，如 winmm）
# -----------------------------------------------------------------------------
function(_gear_demo_resolve_libs OUT_VAR)
    set(_resolved "")
    foreach(_lib IN LISTS ARGN)
        if(IS_ABSOLUTE "${_lib}")
            list(APPEND _resolved "${_lib}")
            continue()
        endif()

        set(_name "${_lib}")
        string(REGEX REPLACE "\\.[Ll][Ii][Bb]$" "" _name "${_name}")

        set(_release "${GEAR_LIBS_DIR}/${_name}.lib")
        set(_debug   "${GEAR_LIBS_DIR}/${_name}-d.lib")
        if(EXISTS "${_release}" AND EXISTS "${_debug}")
            list(APPEND _resolved
                "$<$<CONFIG:Debug>:${_debug}>"
                "$<$<NOT:$<CONFIG:Debug>>:${_release}>")
        elseif(EXISTS "${_release}")
            list(APPEND _resolved "${_release}")
        elseif(EXISTS "${_debug}")
            list(APPEND _resolved "$<$<CONFIG:Debug>:${_debug}>")
        else()
            list(APPEND _resolved "${_lib}")
        endif()
    endforeach()
    set(${OUT_VAR} "${_resolved}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# 运行期依赖检查（Demo 构建前执行）
#
# Libs 里的导入库必须与 Runtime/Dlls 里的 DLL 配套，否则运行时加载失败；
# 这条隐式约定很容易在后续引入新第三方库时被遗忘，因此每次 Libs/Dlls
# 发生变化时自动检查一遍（见 cmake/GearCheckRuntimeDlls.cmake）。
# 检查结果用 stamp 文件记录，没有变化时不会重复执行。
# -----------------------------------------------------------------------------
set(GEAR_DUMPBIN_EXECUTABLE "")
if(CMAKE_LINKER)
    get_filename_component(_gear_link_dir "${CMAKE_LINKER}" DIRECTORY)
    if(EXISTS "${_gear_link_dir}/dumpbin.exe")
        set(GEAR_DUMPBIN_EXECUTABLE "${_gear_link_dir}/dumpbin.exe")
    endif()
endif()
if(NOT GEAR_DUMPBIN_EXECUTABLE)
    find_program(GEAR_DUMPBIN_EXECUTABLE dumpbin)
endif()

if(GEAR_DUMPBIN_EXECUTABLE)
    set(_gear_check_stamp "${CMAKE_BINARY_DIR}/GearCheckRuntimeDlls.stamp")
    # 不用 CONFIGURE_DEPENDS：新库仍需重新 configure 才会被链接，这里保持一致
    file(GLOB_RECURSE _gear_check_libs "${GEAR_LIBS_DIR}/*.lib")
    file(GLOB_RECURSE _gear_check_dlls "${GEAR_DLLS_DIR}/*.dll")

    add_custom_command(
        OUTPUT "${_gear_check_stamp}"
        COMMAND "${CMAKE_COMMAND}"
                "-DGEAR_LIBS_DIR=${GEAR_LIBS_DIR}"
                "-DGEAR_DLLS_DIR=${GEAR_DLLS_DIR}"
                "-DGEAR_DUMPBIN_EXECUTABLE=${GEAR_DUMPBIN_EXECUTABLE}"
                -P "${GEAR_ROOT_DIR}/cmake/GearCheckRuntimeDlls.cmake"
        COMMAND "${CMAKE_COMMAND}" -E touch "${_gear_check_stamp}"
        DEPENDS ${_gear_check_libs} ${_gear_check_dlls}
                "${GEAR_ROOT_DIR}/cmake/GearCheckRuntimeDlls.cmake"
        COMMENT "检查 Libs 导入库与 Runtime/Dlls 的配套关系"
        VERBATIM)

    add_custom_target(GearCheckRuntimeDlls DEPENDS "${_gear_check_stamp}")
else()
    add_custom_target(GearCheckRuntimeDlls)
    message(STATUS "未找到 dumpbin.exe，跳过运行期依赖检查")
endif()

# -----------------------------------------------------------------------------
# 内部：Runtime/Dlls 下所有 DLL -> /DELAYLOAD:xxx.dll
# -----------------------------------------------------------------------------
function(_gear_demo_delayload OUT_VAR)
    file(GLOB _dlls "${GEAR_DLLS_DIR}/*.dll")
    set(_options "")
    foreach(_dll IN LISTS _dlls)
        get_filename_component(_name "${_dll}" NAME)
        list(APPEND _options "/DELAYLOAD:${_name}")
    endforeach()
    set(${OUT_VAR} "${_options}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# 定义一个示例项目
#
#   gear_add_demo(<target>
#       [SOURCES ...] [HEADERS ...]          # 省略则取当前目录 *.cpp / *.h
#       [SHADERS "相对路径|类型" ...]         # 相对 PrecompileShaders；类型见 GearShader.cmake
#       [SHADER_MODE CSO|HEADER]             # 默认 CSO
#       [SHADER_OUTPUT_DIR <目录>]           # 仅 CSO 使用，默认 <输出目录>/Assets/Shaders
#       [ASSETS_DIR <目录>]                  # 默认 "Assets"
#       [SUBSYSTEM Console|Windows]          # 默认 Console
#       [ENTRY <符号>]                       # 对应 EntryPointSymbol，如 wmainCRTStartup
#       [OUTPUT_DIR <目录>]                  # 默认 ${GEAR_OUTPUT_ROOT}/<target>/$<CONFIG>
#       [LIBS ...] [INCLUDE_DIRS ...] [DEFINITIONS ...]
#       [SHADER_INCLUDE_DIRS ...] [SHADER_DEFINITIONS ...]
#       [NO_ASSET_COPY]                      # 不拷贝 Demo 的 Assets 与 Runtime/Assets
#   )
# -----------------------------------------------------------------------------
function(gear_add_demo TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 A
        "NO_ASSET_COPY"
        "SUBSYSTEM;ENTRY;OUTPUT_DIR;ASSETS_DIR;SHADER_MODE;SHADER_OUTPUT_DIR"
        "SOURCES;HEADERS;SHADERS;LIBS;INCLUDE_DIRS;DEFINITIONS;SHADER_INCLUDE_DIRS;SHADER_DEFINITIONS")

    # ---- 源文件 ----
    if(A_SOURCES)
        set(_sources ${A_SOURCES})
    else()
        file(GLOB _sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    endif()
    if(A_HEADERS)
        set(_headers ${A_HEADERS})
    else()
        file(GLOB _headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    endif()
    if(NOT _sources)
        message(FATAL_ERROR "gear_add_demo: ${TARGET} 没有源文件")
    endif()

    # ---- 可执行文件 ----
    if(A_SUBSYSTEM)
        string(TOUPPER "${A_SUBSYSTEM}" _subsystem)
    else()
        set(_subsystem CONSOLE)
    endif()
    if(_subsystem STREQUAL "WINDOWS")
        add_executable(${TARGET} WIN32 ${_sources} ${_headers})
    else()
        add_executable(${TARGET} ${_sources} ${_headers})
    endif()

    # ---- 头文件搜索路径（对应 IncludePath） ----
    target_include_directories(${TARGET} PRIVATE
        "${GEAR_ENGINE_DIR}/External"
        "${GEAR_ENGINE_DIR}/Inc"
        ${A_INCLUDE_DIRS}
        ${GEAR_DEMO_INCLUDE_DIRS}
    )

    # ---- 预处理器定义 ----
    target_compile_definitions(${TARGET} PRIVATE
        _CONSOLE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        ${A_DEFINITIONS}
        ${GEAR_DEMO_DEFINITIONS}
    )

    # ---- 链接 ----
    # Libs 下的第三方静态库由 GearStaticLibs 统一提供（自动 -d 配对）
    _gear_demo_resolve_libs(_extra_libs ${GEAR_DEMO_LIBS} ${A_LIBS})

    target_link_libraries(${TARGET} PRIVATE
        Engine
        GearStaticLibs
        ${_extra_libs}
        D3D12 DXGI dxguid delayimp
    )

    # 构建前先确保 Libs 里的导入库都有配套 DLL（检查失败直接中止构建）
    add_dependencies(${TARGET} GearCheckRuntimeDlls)

    # MSBuild 侧 Release 也开了 GenerateDebugInformation
    target_link_options(${TARGET} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
    if(A_ENTRY)
        target_link_options(${TARGET} PRIVATE "/ENTRY:${A_ENTRY}")
    endif()

    # 对应 DemoCommon.props：Dlls 下所有 DLL 全部延迟加载
    _gear_demo_delayload(_delayload)
    if(_delayload)
        target_link_options(${TARGET} PRIVATE ${_delayload})
    endif()

    # ---- 输出目录（对应 OutDir） ----
    set(_out_dir "${A_OUTPUT_DIR}")
    if(NOT _out_dir)
        set(_out_dir "${GEAR_OUTPUT_ROOT}/${TARGET}/$<CONFIG>")
    endif()
    if(NOT IS_ABSOLUTE "${_out_dir}")
        set(_out_dir "${CMAKE_CURRENT_SOURCE_DIR}/${_out_dir}")
    endif()
    file(TO_CMAKE_PATH "${_out_dir}" _out_dir)
    set_target_properties(${TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${_out_dir}"
    )

    # ---- 运行期资源与 DLL（对应 PostBuildEvent 的 xcopy） ----
    if(NOT A_NO_ASSET_COPY)
        set(_assets "${A_ASSETS_DIR}")
        if(NOT _assets)
            set(_assets "Assets")
        endif()
        if(NOT IS_ABSOLUTE "${_assets}")
            set(_assets "${CMAKE_CURRENT_SOURCE_DIR}/${_assets}")
        endif()

        if(EXISTS "${_assets}")
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${_assets}" "$<TARGET_FILE_DIR:${TARGET}>/Assets"
                COMMENT "复制 ${TARGET} 的 Assets"
                VERBATIM)
        endif()
        # Runtime/Assets 下按子目录组织（如 EngineAssets），整体并入输出目录的 Assets
        if(EXISTS "${GEAR_RUNTIME_ASSETS_DIR}")
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${GEAR_RUNTIME_ASSETS_DIR}" "$<TARGET_FILE_DIR:${TARGET}>/Assets"
                COMMENT "复制 Runtime/Assets"
                VERBATIM)
        endif()
    endif()

    # 引擎公共着色器头：运行时编译的着色器可能 include 它们。
    # 保留 ShaderCommon 目录名，输出到 <程序目录>/Assets/Shaders/ShaderCommon。
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${GEAR_ENGINE_DIR}/PrecompileShaders/ShaderCommon"
                "$<TARGET_FILE_DIR:${TARGET}>/Assets/Shaders/ShaderCommon"
        COMMENT "复制 ShaderCommon"
        VERBATIM)

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${GEAR_DLLS_DIR}" "$<TARGET_FILE_DIR:${TARGET}>/Dlls"
        COMMENT "复制 Dlls"
        VERBATIM)

    # ---- 着色器（对应 FxCompile + ObjectFileOutput） ----
    if(A_SHADERS)
        set(_shader_mode "${A_SHADER_MODE}")
        if(NOT _shader_mode)
            set(_shader_mode CSO)
        endif()

        set(_shader_out "${A_SHADER_OUTPUT_DIR}")
        if(NOT _shader_out)
            set(_shader_out "${_out_dir}/Assets/Shaders")
        endif()

        # 在函数作用域内设置项目默认，只影响下面这几个 gear_add_shader 调用。
        # 产物实际写入 _shader_out/CompiledShaders，与运行时
        # CompiledShaderPath（Assets/Shaders/CompiledShaders/）一致。
        gear_shader_defaults(
            MODE ${_shader_mode}
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PrecompileShaders"
            OUTPUT_DIR "${_shader_out}"
            INCLUDE_DIRS ${A_SHADER_INCLUDE_DIRS} ${GEAR_DEMO_SHADER_INCLUDE_DIRS}
            DEFINITIONS ${A_SHADER_DEFINITIONS} ${GEAR_DEMO_SHADER_DEFINITIONS}
        )

        foreach(_entry IN LISTS A_SHADERS)
            string(REPLACE "|" ";" _parts "${_entry}")
            list(LENGTH _parts _count)
            if(NOT _count EQUAL 2)
                message(FATAL_ERROR
                    "gear_add_demo: 着色器条目格式应为 \"相对路径|类型\"，收到 \"${_entry}\"（${TARGET}）")
            endif()
            list(GET _parts 0 _source)
            list(GET _parts 1 _type)
            gear_add_shader(${TARGET} "${_source}" TYPE ${_type})
        endforeach()
    endif()
endfunction()

# =============================================================================
# GearDemo.cmake —— 示例项目（Demos/*、TODOAPP）的公共构建规则
#
# 对应 SampleProject.vcxproj + Demos/DemoCommon.props：
#   IncludePath                  -> Engine/External、Engine/Inc
#   LibraryPath                  -> Libs（由 _gear_demo_resolve_libs 解析）
#   OutDir                       -> Build/<项目>/<Config>
#   WarningLevel/SDLCheck/字符集 -> 顶层统一（/W3 默认，另有 /sdl /utf-8 /permissive-）
#   PreprocessorDefinitions      -> _CONSOLE + _DEBUG/NDEBUG
#   AdditionalDependencies       -> 基础库（自动处理 -d 后缀）+ LIBS 追加
#   PostBuildEvent(xcopy)        -> Assets / EngineAssets / Dlls 三份拷贝
#   FxCompile(ObjectFileOutput)  -> Assets/<名>.cso（转交 GearShader.cmake）
#   DemoCommon.props(/DELAYLOAD) -> 自动收集 Dlls/*.dll 生成 /DELAYLOAD
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
# =============================================================================

if(NOT DEFINED GEAR_DXC_EXECUTABLE)
    message(FATAL_ERROR "GearDemo.cmake 需要先由顶层 CMakeLists 完成着色器相关初始化")
endif()

# 所有示例项目都会链接的基础库（名字不带 .lib；存在 <名>-d.lib 时 Debug 自动用它）
set(_GEAR_DEMO_BASE_LIBS
    DirectXTex
    assimp-vc145-mt
    OpenEXR-3_4
    GFSDK_SSAO_D3D12.win64
    avcodec
    avformat
    avutil
    swresample
    swscale
    dxcompiler
    dxil
)

# -----------------------------------------------------------------------------
# 内部：把库名解析成 Libs 下的绝对路径，Debug 优先使用 -d 版本
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
        if(EXISTS "${_debug}" AND EXISTS "${_release}")
            list(APPEND _resolved "$<$<CONFIG:Debug>:${_debug}>" "$<$<CONFIG:Release>:${_release}>")
        elseif(EXISTS "${_release}")
            list(APPEND _resolved "${_release}")
        else()
            message(FATAL_ERROR "gear_add_demo: 找不到库 ${_name}.lib（${GEAR_LIBS_DIR}）")
        endif()
    endforeach()
    set(${OUT_VAR} "${_resolved}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# 内部：Dlls 下所有 DLL -> /DELAYLOAD:xxx.dll
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
#       [SHADERS "相对路径|类型" ...]         # 类型见 GearShader.cmake
#       [SHADER_MODE CSO|HEADER]             # 默认 CSO
#       [SHADER_OUTPUT_DIR <目录>]           # 默认 <输出目录>/Assets
#       [ASSETS_DIR <目录>]                  # 默认 "Assets"
#       [SUBSYSTEM Console|Windows]          # 默认 Console
#       [ENTRY <符号>]                       # 对应 EntryPointSymbol，如 wmainCRTStartup
#       [OUTPUT_DIR <目录>]                  # 默认 ${GEAR_OUTPUT_ROOT}/<target>/$<CONFIG>
#       [LIBS ...] [INCLUDE_DIRS ...] [DEFINITIONS ...]
#       [SHADER_INCLUDE_DIRS ...] [SHADER_DEFINITIONS ...]
#       [NO_ASSET_COPY]                      # 不拷贝 Assets / EngineAssets
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
    _gear_demo_resolve_libs(_base_libs ${_GEAR_DEMO_BASE_LIBS})
    _gear_demo_resolve_libs(_extra_libs ${GEAR_DEMO_LIBS} ${A_LIBS})

    target_link_libraries(${TARGET} PRIVATE
        Engine
        ${_base_libs}
        ${_extra_libs}
        D3D12 DXGI dxguid delayimp
    )

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
        if(EXISTS "${GEAR_ROOT_DIR}/EngineAssets")
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${GEAR_ROOT_DIR}/EngineAssets" "$<TARGET_FILE_DIR:${TARGET}>/Assets/EngineAssets"
                COMMENT "复制 EngineAssets"
                VERBATIM)
        endif()
    endif()

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
            set(_shader_out "${_out_dir}/Assets")
        endif()

        # 在函数作用域内设置项目默认，只影响下面这几个 gear_add_shader 调用
        gear_shader_defaults(
            MODE ${_shader_mode}
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

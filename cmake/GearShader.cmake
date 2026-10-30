# =============================================================================
# GearShader.cmake —— 统一的 HLSL 着色器编译规则
#
# 三层配置模型：
#   全局       GEAR_SHADER_MODEL / GEAR_DXC_EXECUTABLE / 两个开关
#   项目级     gear_shader_defaults(...)   作用域 = 调用目录，子目录可继承
#   着色器级   gear_add_shader(...)        继承项目默认，可追加 include/定义、覆盖模式
#
# 目录约定：
#   源文件全部位于 SOURCE_DIR（默认 <当前目录>/PrecompileShaders），
#   gear_add_shader 的 <源文件> 参数相对 SOURCE_DIR。
#   产物统一放在 <OUTPUT_DIR>/CompiledShaders/ 下，并镜像源文件的相对路径，
#   因此不同子目录可以安全地出现同名着色器。
#
# 与原 MSBuild FxCompile 的对应关系：
#   AdditionalIncludeDirectories -> -I
#   ShaderType                   -> -T <profile>_<model>
#   PreprocessorDefinitions      -> -D（Debug 恒带 _DEBUG）
#   HeaderFileOutput/VariableName-> -Fh + -Vn g_<名>Bytes
#   ObjectFileOutput             -> -Fo
#   /nologo                      -> -nologo
#       （实测 dxc 同时接受 /nologo 与 -nologo，产物逐字节相同，这里与其它参数统一风格）
#
# 模式优先级：
#   着色器级 MODE > Lib 类型默认 CSO > 项目级 MODE > 报错
#
# 增量语义：
#   HEADER 模式先输出到构建目录下的临时文件，再 copy_if_different 到最终路径。
#   内容不变时不刷新 mtime，配合 CMake 自动生成的 restat 规则，依赖该头的
#   .cpp 不会重编，更不会触发重链接。
# =============================================================================

if(NOT DEFINED GEAR_DXC_EXECUTABLE)
    message(FATAL_ERROR "GearShader.cmake 需要先设置 GEAR_DXC_EXECUTABLE（应在顶层 CMakeLists 中查找）")
endif()

if(NOT DEFINED GEAR_ENGINE_DIR)
    message(FATAL_ERROR "GearShader.cmake 需要先设置 GEAR_ENGINE_DIR")
endif()

if(NOT DEFINED GEAR_SHADER_MODEL)
    set(GEAR_SHADER_MODEL "6.6")
endif()

# 所有着色器都强制附加的搜索目录（对应 vcxproj 的
# $(SolutionDir)Engine\PrecompileShaders\ShaderCommon）
set(GEAR_SHADER_FORCED_INCLUDES "${GEAR_ENGINE_DIR}/PrecompileShaders/ShaderCommon")
if(NOT EXISTS "${GEAR_SHADER_FORCED_INCLUDES}")
    message(FATAL_ERROR "缺少着色器公共头目录：${GEAR_SHADER_FORCED_INCLUDES}")
endif()

set(GEAR_SHADER_OUTPUT_IN_BUILD_TREE OFF CACHE BOOL
    "着色器产物输出到构建目录而不是源码目录；用于彻底隔离 Debug/Release 的生成物")
set(GEAR_SHADER_AUTO_DEPS ON CACHE BOOL
    "自动把 SOURCE_DIR 与全局强制 include 目录下的 *.hlsli 收集为依赖；不收集项目自定义 include 目录")

# 着色器类型 -> dxc target 前缀（与 vcxproj 的 ShaderType 一致）。
# 这些是文件级普通变量，函数内可见。
set(_GEAR_SHADER_PROFILE_Vertex        vs)
set(_GEAR_SHADER_PROFILE_Hull          hs)
set(_GEAR_SHADER_PROFILE_Domain        ds)
set(_GEAR_SHADER_PROFILE_Geometry      gs)
set(_GEAR_SHADER_PROFILE_Pixel         ps)
set(_GEAR_SHADER_PROFILE_Compute       cs)
set(_GEAR_SHADER_PROFILE_Amplification as)
set(_GEAR_SHADER_PROFILE_Mesh          ms)
set(_GEAR_SHADER_PROFILE_Lib           lib)

# -----------------------------------------------------------------------------
# 内部：收集指定目录下的 *.hlsli（按目录缓存，避免每个着色器重复扫描）
# -----------------------------------------------------------------------------
function(_gear_shader_collect_hlsli OUT_VAR)
    set(_files "")
    foreach(_dir IN LISTS ARGN)
        if(NOT _dir)
            continue()
        endif()
        get_filename_component(_abs "${_dir}" ABSOLUTE)
        string(MAKE_C_IDENTIFIER "${_abs}" _key)
        get_property(_cached GLOBAL PROPERTY "GEAR_SHADER_HLSLI_${_key}")
        if(NOT _cached)
            file(GLOB_RECURSE _cached CONFIGURE_DEPENDS "${_abs}/*.hlsli")
            set_property(GLOBAL PROPERTY "GEAR_SHADER_HLSLI_${_key}" "${_cached}")
        endif()
        list(APPEND _files ${_cached})
    endforeach()

    if(_files)
        list(REMOVE_DUPLICATES _files)
        list(SORT _files)
    endif()
    set(${OUT_VAR} "${_files}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# 项目级默认设置（作用于调用目录，子目录 add_subdirectory 会继承）
#
#   gear_shader_defaults(
#       MODE        HEADER|CSO      # 可省略：省略时每个着色器必须自己指定
#       SOURCE_DIR  <目录>          # 可省略：省略则用 <当前目录>/PrecompileShaders
#       OUTPUT_DIR  <目录>          # 仅 CSO 使用；可省略，省略则用 SOURCE_DIR
#                                   # CSO 产物写入 <OUTPUT_DIR>/CompiledShaders
#                                   # HEADER 固定写入 <SOURCE_DIR>/CompiledShaders
#       INCLUDE_DIRS ...            # 追加
#       DEFINITIONS  ...            # 追加
#   )
# -----------------------------------------------------------------------------
function(gear_shader_defaults)
    cmake_parse_arguments(PARSE_ARGV 0 A "" "MODE;SOURCE_DIR;OUTPUT_DIR" "INCLUDE_DIRS;DEFINITIONS")

    if(DEFINED A_MODE AND NOT A_MODE STREQUAL "")
        string(TOUPPER "${A_MODE}" _mode)
        if(NOT _mode STREQUAL "HEADER" AND NOT _mode STREQUAL "CSO")
            message(FATAL_ERROR "gear_shader_defaults: MODE 只能是 HEADER 或 CSO，收到 \"${A_MODE}\"")
        endif()
        set(GEAR_SHADER_DEFAULT_MODE "${_mode}" PARENT_SCOPE)
    endif()

    if(DEFINED A_SOURCE_DIR AND NOT A_SOURCE_DIR STREQUAL "")
        set(GEAR_SHADER_SOURCE_DIR "${A_SOURCE_DIR}" PARENT_SCOPE)
    endif()

    if(DEFINED A_OUTPUT_DIR AND NOT A_OUTPUT_DIR STREQUAL "")
        set(GEAR_SHADER_OUTPUT_DIR "${A_OUTPUT_DIR}" PARENT_SCOPE)
    endif()

    if(A_INCLUDE_DIRS)
        set(_merged "${GEAR_SHADER_INCLUDE_DIRS}")
        list(APPEND _merged ${A_INCLUDE_DIRS})
        list(REMOVE_DUPLICATES _merged)
        set(GEAR_SHADER_INCLUDE_DIRS "${_merged}" PARENT_SCOPE)
    endif()

    if(A_DEFINITIONS)
        set(_merged "${GEAR_SHADER_DEFINITIONS}")
        list(APPEND _merged ${A_DEFINITIONS})
        list(REMOVE_DUPLICATES _merged)
        set(GEAR_SHADER_DEFINITIONS "${_merged}" PARENT_SCOPE)
    endif()
endfunction()

# -----------------------------------------------------------------------------
# 编译单个着色器
#
#   gear_add_shader(<target> <源文件>
#       TYPE        Vertex|Hull|Domain|Geometry|Pixel|Compute|Amplification|Mesh|Lib
#       [MODE       HEADER|CSO]     # 覆盖项目默认；项目未指定时必填
#       [SOURCE_DIR <目录>]         # 覆盖项目默认源根
#       [OUTPUT_DIR <目录>]         # 覆盖项目默认输出根
#       [INCLUDE_DIRS ...]          # 追加
#       [DEFINITIONS  ...]          # 追加
#   )
#
# <源文件> 是相对 SOURCE_DIR 的路径，例如 "Bloom/BloomFilterPS.hlsl"。
# 产物写入各自根目录下的 CompiledShaders/，并镜像源文件的相对路径：
#   HEADER -> <SOURCE_DIR>/CompiledShaders/<相对路径>.h（忽略 OUTPUT_DIR）
#   CSO    -> <OUTPUT_DIR>/CompiledShaders/<相对路径>.cso（OUTPUT_DIR 默认 SOURCE_DIR）
# GEAR_SHADER_OUTPUT_IN_BUILD_TREE=ON 时改为镜像到构建树。
# OUTPUT_DIR 相对路径默认相对当前源码目录；GEAR_SHADER_OUTPUT_IN_BUILD_TREE=ON 时相对构建目录。
# -----------------------------------------------------------------------------
function(gear_add_shader TARGET SOURCE)
    cmake_parse_arguments(PARSE_ARGV 2 A "" "TYPE;MODE;SOURCE_DIR;OUTPUT_DIR" "INCLUDE_DIRS;DEFINITIONS")

    if(NOT TARGET ${TARGET})
        message(FATAL_ERROR "gear_add_shader: 目标不存在：${TARGET}")
    endif()

    if(NOT A_TYPE)
        message(FATAL_ERROR "gear_add_shader: 缺少 TYPE（${SOURCE}）")
    endif()

    set(_profile "${_GEAR_SHADER_PROFILE_${A_TYPE}}")
    if(NOT _profile)
        message(FATAL_ERROR
            "gear_add_shader: 未知着色器类型 \"${A_TYPE}\"（${SOURCE}）。\n"
            "可用类型：Vertex Hull Domain Geometry Pixel Compute Amplification Mesh Lib")
    endif()

    # ---- 模式：着色器级 > Lib 默认 CSO > 项目级 > 报错 ----
    if(DEFINED A_MODE AND NOT A_MODE STREQUAL "")
        string(TOUPPER "${A_MODE}" _mode)
    elseif(_profile STREQUAL "lib")
        set(_mode CSO)
    elseif(DEFINED GEAR_SHADER_DEFAULT_MODE AND NOT GEAR_SHADER_DEFAULT_MODE STREQUAL "")
        set(_mode "${GEAR_SHADER_DEFAULT_MODE}")
    else()
        message(FATAL_ERROR
            "gear_add_shader: ${SOURCE} 没有可用的输出模式。\n"
            "请用 gear_shader_defaults(MODE ...) 设置项目默认，或在此处显式指定 MODE。")
    endif()
    if(NOT _mode STREQUAL "HEADER" AND NOT _mode STREQUAL "CSO")
        message(FATAL_ERROR "gear_add_shader: MODE 只能是 HEADER 或 CSO，收到 \"${A_MODE}\"（${SOURCE}）")
    endif()

    if(_mode STREQUAL "HEADER")
        get_target_property(_target_type ${TARGET} TYPE)
        if(_target_type STREQUAL "UTILITY")
            message(FATAL_ERROR
                "gear_add_shader: HEADER 模式需要库或可执行目标，"
                "${TARGET} 是自定义目标（${SOURCE}）。\n"
                "自定义目标请改用 MODE CSO。")
        endif()
    endif()

    # ---- 路径 ----
    # 源根：SOURCE 是相对源根的路径，默认 <当前目录>/PrecompileShaders
    set(_source_root "${A_SOURCE_DIR}")
    if(NOT _source_root)
        set(_source_root "${GEAR_SHADER_SOURCE_DIR}")
    endif()
    if(NOT _source_root)
        set(_source_root "${CMAKE_CURRENT_SOURCE_DIR}/PrecompileShaders")
    endif()
    if(NOT IS_ABSOLUTE "${_source_root}")
        set(_source_root "${CMAKE_CURRENT_SOURCE_DIR}/${_source_root}")
    endif()
    file(TO_CMAKE_PATH "${_source_root}" _source_root)
    get_filename_component(_source_root "${_source_root}" ABSOLUTE)
    if(NOT EXISTS "${_source_root}")
        message(FATAL_ERROR "gear_add_shader: 源目录不存在：${_source_root}（${SOURCE}）")
    endif()

    if(IS_ABSOLUTE "${SOURCE}")
        set(_src "${SOURCE}")
    else()
        set(_src "${_source_root}/${SOURCE}")
    endif()
    file(TO_CMAKE_PATH "${_src}" _src)
    get_filename_component(_src "${_src}" ABSOLUTE)
    if(NOT EXISTS "${_src}")
        message(FATAL_ERROR "gear_add_shader: 着色器源文件不存在：${_src}")
    endif()

    file(RELATIVE_PATH _rel "${_source_root}" "${_src}")
    if(_rel MATCHES "^\\.\\.")
        message(FATAL_ERROR
            "gear_add_shader: 源文件不在 SOURCE_DIR 下：${_src}\nSOURCE_DIR = ${_source_root}")
    endif()
    get_filename_component(_rel_dir "${_rel}" DIRECTORY)
    get_filename_component(_name "${_rel}" NAME_WE)
    if(_rel_dir)
        set(_rel_prefix "${_rel_dir}/")
    else()
        set(_rel_prefix "")
    endif()

    # 输出根：
    #   HEADER 固定写入 SOURCE_DIR，忽略 OUTPUT_DIR
    #   CSO    写入 OUTPUT_DIR，未指定时回退 SOURCE_DIR
    if(_mode STREQUAL "HEADER")
        if(GEAR_SHADER_OUTPUT_IN_BUILD_TREE)
            # 产物镜像到构建树中与源根同名的目录
            get_filename_component(_source_root_name "${_source_root}" NAME)
            set(_out_abs "${CMAKE_CURRENT_BINARY_DIR}/${_source_root_name}")
        else()
            set(_out_abs "${_source_root}")
        endif()
    else()
        set(_out_dir "${A_OUTPUT_DIR}")
        if(NOT _out_dir)
            set(_out_dir "${GEAR_SHADER_OUTPUT_DIR}")
        endif()
        if(NOT _out_dir)
            if(GEAR_SHADER_OUTPUT_IN_BUILD_TREE)
                # 默认输出到构建树，并保持与源码树一致的目录名
                get_filename_component(_source_root_name "${_source_root}" NAME)
                set(_out_dir "${_source_root_name}")
            else()
                set(_out_dir "${_source_root}")
            endif()
        endif()

        if(IS_ABSOLUTE "${_out_dir}")
            set(_out_abs "${_out_dir}")
        elseif(GEAR_SHADER_OUTPUT_IN_BUILD_TREE)
            set(_out_abs "${CMAKE_CURRENT_BINARY_DIR}/${_out_dir}")
        else()
            set(_out_abs "${CMAKE_CURRENT_SOURCE_DIR}/${_out_dir}")
        endif()
        file(TO_CMAKE_PATH "${_out_abs}" _out_abs)
        get_filename_component(_out_abs "${_out_abs}" ABSOLUTE)
    endif()

    set(_out_compiled "${_out_abs}/CompiledShaders")

    if(_mode STREQUAL "HEADER")
        set(_final "${_out_compiled}/${_rel_prefix}${_name}.h")
    elseif(_profile STREQUAL "lib")
        set(_final "${_out_compiled}/${_rel_prefix}${_name}.lib")
    else()
        set(_final "${_out_compiled}/${_rel_prefix}${_name}.cso")
    endif()

    get_property(_owner GLOBAL PROPERTY "GEAR_SHADER_OUTPUT_${_final}")
    if(_owner AND NOT _owner STREQUAL "${TARGET}")
        message(FATAL_ERROR
            "gear_add_shader: 输出冲突 ${_final}\n"
            "已被目标 ${_owner} 占用（当前目标 ${TARGET}，源 ${SOURCE}）。")
    endif()
    set_property(GLOBAL PROPERTY "GEAR_SHADER_OUTPUT_${_final}" "${TARGET}")

    # ---- 附加包含目录：着色器级 -> 项目级 -> 强制目录（具体优先） ----
    # 相对路径一律按当前源码目录解析：custom command 的工作目录是构建目录，
    # 直接把相对路径交给 dxc 会找错位置。
    set(_includes "")
    foreach(_dir IN ITEMS ${A_INCLUDE_DIRS} ${GEAR_SHADER_INCLUDE_DIRS} ${GEAR_SHADER_FORCED_INCLUDES})
        if(NOT IS_ABSOLUTE "${_dir}")
            set(_dir "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}")
        endif()
        file(TO_CMAKE_PATH "${_dir}" _dir)
        get_filename_component(_dir "${_dir}" ABSOLUTE)
        list(APPEND _includes "${_dir}")
    endforeach()
    if(_includes)
        list(REMOVE_DUPLICATES _includes)
    endif()

    # ---- 预处理器定义：_DEBUG -> 项目级 -> 着色器级 ----
    set(_defs ${GEAR_SHADER_DEFINITIONS} ${A_DEFINITIONS})
    if(_defs)
        list(REMOVE_DUPLICATES _defs)
    endif()

    # ---- 命令行 ----
    # dxc 的 target 写作 ps_6_6，而 GEAR_SHADER_MODEL 是给人读的 6.6
    string(REPLACE "." "_" _model_suffix "${GEAR_SHADER_MODEL}")
    set(_dxc "${GEAR_DXC_EXECUTABLE}")
    list(APPEND _dxc -T "${_profile}_${_model_suffix}")
    list(APPEND _dxc -nologo)
    if(NOT _profile STREQUAL "lib")
        list(APPEND _dxc -E main)
    endif()
    foreach(_dir IN LISTS _includes)
        list(APPEND _dxc -I "${_dir}")
    endforeach()

    if(CMAKE_BUILD_TYPE)
        string(TOUPPER "${CMAKE_BUILD_TYPE}" _config)
        if(_config STREQUAL "DEBUG")
            list(APPEND _dxc -D_DEBUG)
        endif()
    else()
        # 多配置生成器：空展开由 COMMAND_EXPAND_LISTS 丢弃
        list(APPEND _dxc "$<$<CONFIG:Debug>:-D_DEBUG>")
    endif()

    foreach(_def IN LISTS _defs)
        list(APPEND _dxc -D "${_def}")
    endforeach()

    set(_tmp_dir "${CMAKE_CURRENT_BINARY_DIR}/shadertmp")
    if(_mode STREQUAL "HEADER")
        set(_tmp "${_tmp_dir}/${_rel_prefix}${_name}.h")
        list(APPEND _dxc -Vn "g_${_name}Bytes" -Fh "${_tmp}")
    else()
        list(APPEND _dxc -Fo "${_final}")
    endif()

    # ---- 依赖 ----
    # 只收集 SOURCE_DIR 与全局强制 include 目录（Engine/PrecompileShaders/ShaderCommon）下的
    # *.hlsli。项目自定义 include 目录不参与收集：Assets/Shaders 里可能有运行时
    # 编译的着色器，它们的 hlsli 不应触发预编译着色器重建。
    set(_deps "${_src}" "${GEAR_DXC_EXECUTABLE}")
    if(GEAR_SHADER_AUTO_DEPS)
        _gear_shader_collect_hlsli(_hlsli "${_source_root}" "${GEAR_SHADER_FORCED_INCLUDES}")
        if(_hlsli)
            list(APPEND _deps ${_hlsli})
        endif()
    endif()

    # ---- 生成规则 ----
    if(_mode STREQUAL "HEADER")
        add_custom_command(
            OUTPUT "${_final}"
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${_out_compiled}/${_rel_dir}" "${_tmp_dir}/${_rel_dir}"
            COMMAND ${_dxc} "${_src}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_tmp}" "${_final}"
            DEPENDS ${_deps}
            COMMENT "HLSL ${_rel} (${A_TYPE} -> HEADER)"
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
        # 挂到目标上：Ninja 保证先生成 .h，再编译包含它的 .cpp
        target_sources(${TARGET} PRIVATE "${_final}")
        # 记录生成头的相对路径与绝对路径，供 gear_shader_apply_header_deps() 使用
        set_property(GLOBAL APPEND PROPERTY "GEAR_SHADER_HEADERS_${TARGET}"
            "${_rel_prefix}${_name}.h|${_final}")
    else()
        add_custom_command(
            OUTPUT "${_final}"
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${_out_compiled}/${_rel_dir}" "${_tmp_dir}/${_rel_dir}"
            COMMAND ${_dxc} "${_src}"
            DEPENDS ${_deps}
            COMMENT "HLSL ${_rel} (${A_TYPE} -> CSO)"
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
        # 镜像相对路径后，不同子目录可能同名，target 名按相对路径生成
        string(MAKE_C_IDENTIFIER "${_rel}" _rel_ident)
        add_custom_target(${TARGET}_shader_${_rel_ident} DEPENDS "${_final}")
        add_dependencies(${TARGET} ${TARGET}_shader_${_rel_ident})
    endif()
endfunction()

# -----------------------------------------------------------------------------
# 为 HEADER 模式补上编译期隐式依赖
#
# target_sources() 只会给编译边加 order-only 依赖（`||`），Ninja 在计划阶段
# 不会因此追踪生成头的重建，会出现“第一次只重建头、第二次才重编 cpp”的问题。
# 这里扫描目标的 .cpp，找出真正 include 了生成头的源文件，给它们设置
# OBJECT_DEPENDS，使生成头成为编译边的隐式依赖（`|`）。这样 .hlsl/.hlsli
# 一变，Ninja 就能在同一次构建里完成“重建头 -> 重编相关 cpp -> 重链接”。
#
# 必须在同一目录、所有 gear_add_shader() 调用之后执行。
# -----------------------------------------------------------------------------
function(gear_shader_apply_header_deps TARGET)
    get_property(_headers GLOBAL PROPERTY "GEAR_SHADER_HEADERS_${TARGET}")
    if(NOT _headers)
        return()
    endif()

    get_target_property(_sources ${TARGET} SOURCES)
    if(NOT _sources)
        return()
    endif()

    foreach(_src IN LISTS _sources)
        if(NOT _src MATCHES "\\.(c|cc|cpp|cxx)$")
            continue()
        endif()
        if(NOT IS_ABSOLUTE "${_src}")
            set(_src "${CMAKE_CURRENT_SOURCE_DIR}/${_src}")
        endif()
        if(NOT EXISTS "${_src}")
            continue()
        endif()

        file(STRINGS "${_src}" _refs
            REGEX "#[ \t]*include[ \t]*[<\"][^>\"]*CompiledShaders/[^>\"]+[>\"]")
        foreach(_ref IN LISTS _refs)
            if(NOT _ref MATCHES "CompiledShaders/([^>\"]+)")
                continue()
            endif()
            set(_ref_rel "${CMAKE_MATCH_1}")
            get_filename_component(_ref_base "${_ref_rel}" NAME)
            foreach(_entry IN LISTS _headers)
                string(REPLACE "|" ";" _pair "${_entry}")
                list(GET _pair 0 _rel_header)
                list(GET _pair 1 _header_file)
                get_filename_component(_rel_base "${_rel_header}" NAME)
                # 优先按完整相对路径匹配；退回文件名以兼容扁平的旧写法
                if(_ref_rel STREQUAL _rel_header OR _ref_base STREQUAL _rel_base)
                    set_property(SOURCE "${_src}" APPEND PROPERTY OBJECT_DEPENDS "${_header_file}")
                endif()
            endforeach()
        endforeach()
    endforeach()
endfunction()

# -----------------------------------------------------------------------------
# 批量版本：每个条目写作 "相对路径|类型"
#
#   gear_add_shaders(Engine
#       "Bloom/BloomFilterPS.hlsl|Pixel"
#       "GammaCorrectCS.hlsl|Compute"
#   )
#
# 需要为单个着色器追加设置时，直接调用 gear_add_shader。
# -----------------------------------------------------------------------------
function(gear_add_shaders TARGET)
    foreach(_entry IN LISTS ARGN)
        string(REPLACE "|" ";" _parts "${_entry}")
        list(LENGTH _parts _count)
        if(NOT _count EQUAL 2)
            message(FATAL_ERROR "gear_add_shaders: 条目格式应为 \"相对路径|类型\"，收到 \"${_entry}\"")
        endif()
        list(GET _parts 0 _source)
        list(GET _parts 1 _type)
        gear_add_shader(${TARGET} "${_source}" TYPE ${_type})
    endforeach()
endfunction()

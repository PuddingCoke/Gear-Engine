# =============================================================================
# Engine：静态库（对应 Engine.vcxproj）
#
# 增量构建策略：
#   1. Common.hlsli / dummyVS.hlsl 变化
#        -> 重新运行 CommonShaderParse 生成 CommonShaderLayout.h
#        -> 重新编译 Shaders 下全部着色器
#   2. 生成结果先写临时文件，再 copy_if_different 到最终路径
#        -> 内容未变时最终文件 mtime 不变
#        -> Ninja 的 restat + MSVC /showIncludes 依赖扫描决定哪些 cpp 需要重编
#   3. .cpp 对头文件的依赖完全由 /showIncludes 自动跟踪，不做强制全量重编
# =============================================================================

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/Gear/*.cpp"
)

list(APPEND ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/External/DDSTextureLoader/DDSTextureLoader12.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGUI/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGUI/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGUI/imgui_impl_dx12.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGUI/imgui_impl_win32.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGUI/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGUI/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/WICTextureLoader/WICTextureLoader12.cpp"
)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Inc/Gear/*.h"
)

# 构建会生成 Inc/Gear/CompiledShaders/*.h；排除它们，避免 CONFIGURE_DEPENDS
# 因生成物变化触发重新 configure（它们通过 /showIncludes 作为依赖被跟踪）。
list(FILTER ENGINE_HEADERS EXCLUDE REGEX "/CompiledShaders/")

add_library(Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# --- 编译设置（对应 Engine.vcxproj）-----------------------------------------
target_include_directories(Engine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Inc"
)

# 对应 ExternalIncludePath + ExternalWarningLevel=Level1
target_include_directories(Engine
    SYSTEM PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/External"
        "${CMAKE_CURRENT_SOURCE_DIR}/External/ffmpeg"
)

target_compile_definitions(Engine
    PRIVATE
        _CONSOLE
        USING_DIRECTX_HEADERS
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

# 对应 WarningLevel=Level4；ExternalWarningLevel 使用 CMake 默认的 /external:W0。
# 按你的要求不加 /MP：Ninja 已按翻译单元并行，/MP 在单文件编译下没有收益。
target_compile_options(Engine
    PRIVATE
        /W4
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/Gy>
        $<$<CONFIG:Release>:/Zi>
)

# 对应 WholeProgramOptimization（Release）
if(GEAR_IPO_SUPPORTED)
    set_property(TARGET Engine PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

# 输出名与位置与原工程一致：
#   Debug   -> $(SolutionDir)Libs\Engine-d.lib / Engine-d.pdb
#   Release -> $(SolutionDir)Libs\Engine.lib   / Engine.pdb
set_target_properties(Engine PROPERTIES
    OUTPUT_NAME "Engine"
    DEBUG_POSTFIX "-d"
    ARCHIVE_OUTPUT_DIRECTORY "${GEAR_LIBS_DIR}"
    COMPILE_PDB_OUTPUT_DIRECTORY "${GEAR_LIBS_DIR}"
    COMPILE_PDB_NAME_DEBUG "Engine-d"
    COMPILE_PDB_NAME_RELEASE "Engine"
)

# --- dxc ---------------------------------------------------------------------
find_program(GEAR_DXC_EXECUTABLE NAMES dxc)
if(NOT GEAR_DXC_EXECUTABLE)
    file(GLOB _dxc_candidates "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/10.*/x64/dxc.exe")
    if(_dxc_candidates)
        list(SORT _dxc_candidates)
        list(GET _dxc_candidates -1 GEAR_DXC_EXECUTABLE)
    endif()
endif()
if(NOT GEAR_DXC_EXECUTABLE)
    message(FATAL_ERROR "未找到 dxc.exe，请通过 -DGEAR_DXC_EXECUTABLE=<path> 指定。")
endif()

# --- CommonShaderLayout.h ----------------------------------------------------
# CommonShaderParse 从自身 exe 目录读取 dummyVS.hlsl / Common.hlsli，
# 因此每次生成前先同步副本（MSBuild 只在工具重链时 PostBuild，可能读到旧文件）。
set(GEAR_LAYOUT_HEADER
    "${CMAKE_CURRENT_SOURCE_DIR}/Inc/Gear/Core/D3D12Core/CommonShaderLayout.h")
set(GEAR_LAYOUT_TMP
    "${CMAKE_CURRENT_BINARY_DIR}/CommonShaderLayout.tmp.h")

add_custom_command(
    OUTPUT "${GEAR_LAYOUT_HEADER}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/CommonShaderParse/dummyVS.hlsl"
            "$<TARGET_FILE_DIR:CommonShaderParse>/dummyVS.hlsl"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Common.hlsli"
            "$<TARGET_FILE_DIR:CommonShaderParse>/Common.hlsli"
    COMMAND "$<TARGET_FILE:CommonShaderParse>" "${GEAR_LAYOUT_TMP}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GEAR_LAYOUT_TMP}" "${GEAR_LAYOUT_HEADER}"
    DEPENDS
        "${CMAKE_SOURCE_DIR}/CommonShaderParse/dummyVS.hlsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Common.hlsli"
        "$<TARGET_FILE:CommonShaderParse>"
    COMMENT "Generating CommonShaderLayout.h"
    VERBATIM
)

add_custom_target(EngineLayout DEPENDS "${GEAR_LAYOUT_HEADER}")

# --- 着色器 ------------------------------------------------------------------
# 文件|类型，类型取自 Engine.vcxproj（ShaderModel 6.6，入口 main）
set(GEAR_ENGINE_SHADERS
    "Shaders/Bloom/BloomDownSamplePS.hlsl|Pixel"
    "Shaders/Bloom/BloomFilterPS.hlsl|Pixel"
    "Shaders/Bloom/BloomFinalPS.hlsl|Pixel"
    "Shaders/Bloom/BloomHBlurCS.hlsl|Compute"
    "Shaders/Bloom/BloomKarisAveragePS.hlsl|Pixel"
    "Shaders/Bloom/BloomVBlurCS.hlsl|Compute"
    "Shaders/FullScreen/FullScreenPS.hlsl|Pixel"
    "Shaders/FullScreen/FullScreenVS.hlsl|Vertex"
    "Shaders/FXAA/ColorToColorLuma.hlsl|Pixel"
    "Shaders/FXAA/FXAA.hlsl|Pixel"
    "Shaders/GammaCorrectCS.hlsl|Compute"
    "Shaders/HDRClampCS.hlsl|Compute"
    "Shaders/PrimitiveBatch/PrimitiveBatchCircleVS.hlsl|Vertex"
    "Shaders/PrimitiveBatch/PrimitiveBatchLineGS.hlsl|Geometry"
    "Shaders/PrimitiveBatch/PrimitiveBatchLineVS.hlsl|Vertex"
    "Shaders/PrimitiveBatch/PrimitiveBatchPS.hlsl|Pixel"
    "Shaders/PrimitiveBatch/PrimitiveBatchRCLineGS.hlsl|Geometry"
    "Shaders/PrimitiveBatch/PrimitiveBatchRCLineVS.hlsl|Vertex"
    "Shaders/SSR/HiZCopyCS.hlsl|Compute"
    "Shaders/SSR/HiZCreateCS.hlsl|Compute"
    "Shaders/SSR/HiZProcessPS.hlsl|Pixel"
    "Shaders/TextureCube/EquirectangularPS.hlsl|Pixel"
    "Shaders/TextureCube/EquirectangularVS.hlsl|Vertex"
    "Shaders/TextureCube/TextureCubeVS.hlsl|Vertex"
    "Shaders/ToneMapCS.hlsl|Compute"
)

# 着色器实际 include 的公共头文件；任一个变化都重编全部着色器
set(GEAR_ENGINE_HLSLI
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Common.hlsli"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/PBRHeader.hlsli"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/FXAA/FXAAHeader.hlsli"
)

set(GEAR_SHADER_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Inc/Gear/CompiledShaders")
set(GEAR_SHADER_TMP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/shadertmp")

set(GEAR_ENGINE_SHADER_HEADERS "")
foreach(_entry IN LISTS GEAR_ENGINE_SHADERS)
    string(REPLACE "|" ";" _parts "${_entry}")
    list(GET _parts 0 _rel)
    list(GET _parts 1 _type)

    get_filename_component(_name "${_rel}" NAME_WE)
    set(_src    "${CMAKE_CURRENT_SOURCE_DIR}/${_rel}")
    set(_tmp    "${GEAR_SHADER_TMP_DIR}/${_name}.h")
    set(_final  "${GEAR_SHADER_OUTPUT_DIR}/${_name}.h")

    if(_type STREQUAL "Pixel")
        set(_profile "ps_6_6")
    elseif(_type STREQUAL "Vertex")
        set(_profile "vs_6_6")
    elseif(_type STREQUAL "Geometry")
        set(_profile "gs_6_6")
    elseif(_type STREQUAL "Compute")
        set(_profile "cs_6_6")
    else()
        message(FATAL_ERROR "未知着色器类型：${_type}（${_rel}）")
    endif()

    add_custom_command(
        OUTPUT "${_final}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${GEAR_SHADER_TMP_DIR}" "${GEAR_SHADER_OUTPUT_DIR}"
        COMMAND "${GEAR_DXC_EXECUTABLE}"
                -T ${_profile}
                -E main
                -Vn "g_${_name}Bytes"
                -I "${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
                -Fh "${_tmp}"
                "${_src}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_tmp}" "${_final}"
        DEPENDS
            "${_src}"
            ${GEAR_ENGINE_HLSLI}
            "${GEAR_DXC_EXECUTABLE}"
        COMMENT "HLSL ${_name} (${_type})"
        VERBATIM
    )

    list(APPEND GEAR_ENGINE_SHADER_HEADERS "${_final}")
endforeach()

add_custom_target(EngineShaders DEPENDS ${GEAR_ENGINE_SHADER_HEADERS})

# 目标级依赖只保证“先生成、后编译”，不会导致全部源文件重编；
# 具体哪些 cpp 重编仍由 /showIncludes 记录的头文件时间戳决定。
add_dependencies(Engine EngineLayout EngineShaders)

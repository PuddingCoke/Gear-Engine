# =============================================================================
# Engine：静态库（对应 Engine.vcxproj）
#
# 增量构建策略：
#   1. CommonShaderLayout.h 由 CommonShaderParse 生成（对应 MSBuild 的 PreBuild）。
#   2. 每个着色器的依赖 = 源文件 + include 目录（含其所在目录）下所有 *.hlsli，
#      由 cmake/GearShader.cmake 自动收集。
#   3. 着色器产物先写临时文件，再 copy_if_different 到最终路径；
#      内容未变时最终文件 mtime 不变，Ninja 的 restat 跳过下游。
#   4. 生成头通过 OBJECT_DEPENDS 成为相关 .cpp 的隐式依赖，
#      .hlsl/.hlsli 一变即可在一次构建内完成“重建头 -> 重编 cpp -> 重链接”。
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
# 项目级默认：全部编译成 C++ 头文件，输出到 Inc/Gear/CompiledShaders/<名>.h。
# 规则（模式/类型/入口/参数映射/增量）见 cmake/GearShader.cmake。
gear_shader_defaults(
    MODE HEADER
    OUTPUT_DIR "Inc/Gear/CompiledShaders"
)

# 文件|类型，类型取自 Engine.vcxproj（着色器模型由顶层 GEAR_SHADER_MODEL 统一）
gear_add_shaders(Engine
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

# 生成头必须先构建、再编译；同时也用于让 Ninja 在一次构建内完成重编与重链。
gear_shader_apply_header_deps(Engine)

# 产物默认输出到源码树，Inc 已是 Engine 的 PUBLIC include 目录。
# 打开 GEAR_SHADER_OUTPUT_IN_BUILD_TREE 后产物落在构建目录，需要把镜像的
# Inc 根目录补进头文件搜索路径。
if(GEAR_SHADER_OUTPUT_IN_BUILD_TREE)
    target_include_directories(Engine BEFORE PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/Inc")
endif()

# 生成的 .h 通过 target_sources 挂到 Engine 上，Ninja 保证先生成后编译；
# 内容未变化时 copy_if_different 不更新 mtime，依赖它的 .cpp 不会重编。
add_dependencies(Engine EngineLayout)

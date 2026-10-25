# =============================================================================
# CommonShaderParse：从 dummyVS.hlsl 的反射信息生成 CommonShaderLayout.h
#
# 对应 CommonShaderParse.vcxproj（仅保留 sln 实际使用的 x64 配置）。
#
# 该工具在运行期从自身 exe 所在目录读取：
#   dummyVS.hlsl、Common.hlsli、dxcompiler.dll、dxil.dll
# 因此 POST_BUILD 需要把这 4 个文件复制到输出目录。
# =============================================================================

add_executable(CommonShaderParse
    main.cpp
    HeaderWriter.h
)

target_include_directories(CommonShaderParse
    PRIVATE
        "${GEAR_ENGINE_DIR}/External"
        "${GEAR_ENGINE_DIR}/Inc"
)

target_compile_definitions(CommonShaderParse
    PRIVATE
        _CONSOLE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

# 原工程 WarningLevel = Level3
target_compile_options(CommonShaderParse PRIVATE /W3)

target_link_libraries(CommonShaderParse PRIVATE "${GEAR_DXCOMPILER_LIB}")

# 原工程 Release 也开启 GenerateDebugInformation
target_link_options(CommonShaderParse PRIVATE "$<$<CONFIG:Release>:/DEBUG>")

# 原工程 Release 开启 WholeProgramOptimization（能力探测在顶层完成）
if(GEAR_IPO_SUPPORTED)
    set_property(TARGET CommonShaderParse PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

# 输出到 Build/CommonShaderParse/<Config>/，与 MSBuild 产物位置一致，
# 便于迁移期间直接对比（Engine 的 PreBuild 也依赖此路径）。
set_target_properties(CommonShaderParse PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${GEAR_OUTPUT_ROOT}/CommonShaderParse/$<CONFIG>"
)

# 运行期输入与 DLL（对应 vcxproj 的 PostBuildEvent）
add_custom_command(TARGET CommonShaderParse POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/dummyVS.hlsl"
        "${GEAR_ENGINE_DIR}/Shaders/Common.hlsli"
        "${GEAR_DLLS_DIR}/dxcompiler.dll"
        "${GEAR_DLLS_DIR}/dxil.dll"
        "$<TARGET_FILE_DIR:CommonShaderParse>"
    COMMENT "复制 CommonShaderParse 运行期依赖"
    VERBATIM
)

# =============================================================================
# SimpleParticleEffect —— 示例项目（对应 SimpleParticleEffect.vcxproj）
#
# 特殊之处：这个示例没有 FxCompile，着色器由引擎在运行时用 DXC 编译
# （Shader::create(L"ParticleVS.hlsl", ...)），hlsl 作为 Assets/Shaders 资源随包拷贝。
# vcxproj 的 PreBuildEvent 会把 Engine/PrecompileShaders/Common.hlsli 同步到 Assets，
# 这里改为构建后直接同步到输出目录，避免改动被 git 跟踪的源码 Assets。
# =============================================================================
gear_add_demo(SimpleParticleEffect)

add_custom_command(TARGET SimpleParticleEffect POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GEAR_ENGINE_DIR}/PrecompileShaders/Common.hlsli"
            "$<TARGET_FILE_DIR:SimpleParticleEffect>/Assets/Shaders/Common.hlsli"
    COMMENT "同步 Common.hlsli 到 Assets"
    VERBATIM)

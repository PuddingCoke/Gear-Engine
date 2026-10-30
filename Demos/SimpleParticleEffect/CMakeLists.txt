# =============================================================================
# SimpleParticleEffect —— 示例项目（对应 SimpleParticleEffect.vcxproj）
#
# 特殊之处：这个示例没有 FxCompile，着色器由引擎在运行时用 DXC 编译
# （Shader::create(L"ParticleVS.hlsl", ...)），hlsl 作为 Assets/Shaders 资源随包拷贝。
# 引擎公共着色器头（ShaderCommon）由 gear_add_demo 统一拷贝到
# <输出目录>/Assets/Shaders/ShaderCommon。
# =============================================================================
gear_add_demo(SimpleParticleEffect)

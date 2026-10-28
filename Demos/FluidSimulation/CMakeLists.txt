# =============================================================================
# FluidSimulation —— 示例项目（对应 FluidSimulation.vcxproj）
#
# Utility.hlsli 与着色器同目录，由 dxc 自动解析，也会被依赖收集覆盖。
# =============================================================================
gear_add_demo(FluidSimulation
    SHADERS
        "ColorAdvectionCS.hlsl|Compute"
        "DivergenceCS.hlsl|Compute"
        "EdgeHighlightCS.hlsl|Compute"
        "GradientSubtractCS.hlsl|Compute"
        "PhongShadeCS.hlsl|Compute"
        "PressureBoundaryCS.hlsl|Compute"
        "PressureCS.hlsl|Compute"
        "PressureResetCS.hlsl|Compute"
        "SplatColorCS.hlsl|Compute"
        "SplatVelocityCS.hlsl|Compute"
        "VelocityAdvectionCS.hlsl|Compute"
        "VelocityBoundaryCS.hlsl|Compute"
        "VorticityConfinementCS.hlsl|Compute"
        "VorticityCS.hlsl|Compute"
)

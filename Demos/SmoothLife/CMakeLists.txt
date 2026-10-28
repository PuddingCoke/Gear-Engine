# =============================================================================
# SmoothLife —— 示例项目（对应 SmoothLife.vcxproj）
# =============================================================================
gear_add_demo(SmoothLife
    SHADERS
        "EvolveCS.hlsl|Compute"
        "VisualizeCS.hlsl|Compute"
        "WhiteNoiseCS.hlsl|Compute"
)

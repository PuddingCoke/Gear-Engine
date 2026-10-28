# =============================================================================
# PBRRendering —— 示例项目（对应 PBRRendering.vcxproj）
# =============================================================================
gear_add_demo(PBRRendering
    SHADERS
        "PBRVS.hlsl|Vertex"
        "PBRPS.hlsl|Pixel"
        "IrradiancePS.hlsl|Pixel"
        "PrefilterVS.hlsl|Vertex"
        "PrefilterPS.hlsl|Pixel"
        "SkyboxPS.hlsl|Pixel"
)

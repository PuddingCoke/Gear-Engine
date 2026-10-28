# =============================================================================
# CrytekSponza —— 示例项目（对应 CrytekSponza.vcxproj）
#
# Utility.hlsli 与着色器同目录，由 dxc 自动解析，也会被依赖收集覆盖。
# =============================================================================
gear_add_demo(CrytekSponza
    SHADERS
        "CubeRenderVS.hlsl|Vertex"
        "CubeRenderPS.hlsl|Pixel"
        "CubeRenderBouncePS.hlsl|Pixel"
        "DeferredVShader.hlsl|Vertex"
        "DeferredPShader.hlsl|Pixel"
        "DeferredFinal.hlsl|Pixel"
        "IrradianceOctahedralEncode.hlsl|Compute"
        "DepthOctahedralEncode.hlsl|Compute"
        "ShadowVS.hlsl|Vertex"
        "SkybosPShader.hlsl|Pixel"
        "SSRCombinePS.hlsl|Pixel"
)

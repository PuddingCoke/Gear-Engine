# =============================================================================
# FFTOcean —— 示例项目（对应 FFTOcean.vcxproj）
#
# Predefine.hlsli 与各着色器同目录，由 dxc 自动解析，也会被依赖收集覆盖。
# =============================================================================
gear_add_demo(FFTOcean
    SHADERS
        "ConjugateCS.hlsl|Compute"
        "DisplacementSpectrumCS.hlsl|Compute"
        "GridDebugVS.hlsl|Vertex"
        "GridDebugPS.hlsl|Pixel"
        "IFFTCS.hlsl|Compute"
        "OceanHS.hlsl|Hull"
        "OceanDS.hlsl|Domain"
        "OceanVS.hlsl|Vertex"
        "OceanPS.hlsl|Pixel"
        "PermutationCS.hlsl|Compute"
        "SpectrumCS.hlsl|Compute"
        "TextureCubePS.hlsl|Pixel"
        "WaveMergeCS.hlsl|Compute"
)

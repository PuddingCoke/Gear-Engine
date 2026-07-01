#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_GREEN_AS_LUMA 0
#define FXAA_QUALITY__PRESET 39

#include"FXAAHeader.hlsli"

#include"Common.hlsli"

cbuffer TextureIndex : register(PER_INVOKE_CONSTANTS)
{
    uint originTextureIndex;
    float fxaaQualityRcpFrame; // unused
    float fxaaQualitySubpix;
    float fxaaQualityEdgeThreshold;
    float fxaaQualityEdgeThresholdMin;
}

static Texture2D<float4> originTexture = ResourceDescriptorHeap[originTextureIndex];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    uint dx, dy;
    originTexture.GetDimensions(dx, dy);
    float2 rcpro = rcp(float2(dx, dy));
    
    FxaaTex t;
    t.tex = originTexture;
    t.smpl = linearClampSampler;

    return FxaaPixelShader(texCoord, 0, t, t, t, rcpro, 0, 0, 0, fxaaQualitySubpix, fxaaQualityEdgeThreshold, fxaaQualityEdgeThresholdMin, 0, 0, 0, 0);
}
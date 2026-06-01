#include"Common.hlsli"

cbuffer BloomParam : register(PER_INVOKE_CONSTANTS)
{
    uint hdrTextureIdx;
    uint bloomTextureIdx;
    uint lensDirtTextureIdx;
    float exposure;
    float gamma;
    float threshold;
    float intensity;
    float softThreshold;
    float lensDirtIntensity;
}

static Texture2D<float4> hdrTexture = ResourceDescriptorHeap[hdrTextureIdx];

static Texture2D<float4> bloomTexture = ResourceDescriptorHeap[bloomTextureIdx];

static Texture2D<float4> lensDirtTexture = ResourceDescriptorHeap[lensDirtTextureIdx];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float3 originColor = hdrTexture.Sample(linearClampSampler, texCoord).rgb;
    
    float3 dirtColor = lensDirtTexture.Sample(linearClampSampler, texCoord).rgb * lensDirtIntensity;
    
    float3 bloomColor = bloomTexture.Sample(linearClampSampler, texCoord).rgb * (float3(1.0, 1.0, 1.0) + dirtColor);
    
    originColor += intensity * bloomColor;
    
    return float4(originColor, 1.0);
}
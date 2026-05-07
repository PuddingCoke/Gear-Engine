#include"Common.hlsli"

cbuffer BloomParam : register(b2)
{
    uint hdrTextureIdx;
    uint bloomTextureIdx;
    uint lensDirtTextureIdx;
    float exposure;
    float gamma;
    float threshold;
    float intensity;
}

static Texture2D<float4> hdrTexture = ResourceDescriptorHeap[hdrTextureIdx];

static Texture2D<float4> bloomTexture = ResourceDescriptorHeap[bloomTextureIdx];

static Texture2D<float4> lensDirtTexture = ResourceDescriptorHeap[lensDirtTextureIdx];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float3 originColor = hdrTexture.Sample(linearClampSampler, texCoord).rgb;
    
    float3 dirtColor = lensDirtTexture.Sample(linearClampSampler, texCoord).rgb;
    
    float3 bloomColor = bloomTexture.Sample(linearClampSampler, texCoord).rgb * (1.0 + dirtColor);
    
    float3 result = lerp(originColor, originColor + bloomColor, intensity);
    
    return float4(result, 1.0);
}
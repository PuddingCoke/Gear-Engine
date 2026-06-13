#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint originTexIndex;
    uint uvVisibilityTexIndex;
    uint gNormalRoughnessTexIndex;
    float exponentA;
    float exponentB;
}

static Texture2D<float4> originTex = ResourceDescriptorHeap[originTexIndex];

static Texture2D<float4> uvVisibilityTex = ResourceDescriptorHeap[uvVisibilityTexIndex];

static Texture2D<float4> gNormalRoughnessTex = ResourceDescriptorHeap[gNormalRoughnessTexIndex];

float roughnessAttenuate(float roughness)
{
    return pow(1.0 - pow(roughness, exponentA), exponentB);
}

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float3 color = originTex.Sample(pointClampSampler, texCoord).rgb;
    
    float3 uvVisibility = uvVisibilityTex.Sample(pointClampSampler, texCoord).xyz;
    
    float roughness = gNormalRoughnessTex.Sample(pointClampSampler, texCoord).w;
    
    float3 reflectedColor = originTex.Sample(linearClampSampler, uvVisibility.xy).rgb * uvVisibility.z * roughnessAttenuate(roughness);
    
    return float4(color + reflectedColor, 1.0f);
}
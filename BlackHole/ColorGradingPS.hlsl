//来自https://www.shadertoy.com/view/lstSRS 作者：sonicether

#include"Common.hlsli"

cbuffer ResourceIndices : register(PER_INVOKE_CONSTANTS)
{
    uint texIndex;
}

static Texture2D<float4> tex = ResourceDescriptorHeap[texIndex];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float3 color = tex.Sample(linearClampSampler, texCoord).rgb;
    
    color = pow(color, float3(1.0 / 1.5, 1.0 / 1.5, 1.0 / 1.5));

    color = lerp(color, color * color * (3.0 - 2.0 * color), float3(1.0, 1.0, 1.0));
    
    color = pow(color, float3(1.3, 1.20, 1.0));

    color = saturate(color * 1.01);
    
    return float4(color, 1.0f);
}
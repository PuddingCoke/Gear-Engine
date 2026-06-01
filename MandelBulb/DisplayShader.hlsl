#include"Common.hlsli"

struct Indices
{
    uint textureIdx;
};

ConstantBuffer<Indices> indicesData : register(DRAWCALLCONSTANTS);

static const Texture2D<float4> tex = ResourceDescriptorHeap[indicesData.textureIdx];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float3 color = tex.Sample(linearClampSampler, texCoord).rgb;
    
    color.rgb = pow(color.rgb, 1.0 / 1.0);
    
    return float4(color, 1.0);
}
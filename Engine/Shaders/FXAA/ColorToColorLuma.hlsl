#include"Common.hlsli"

cbuffer TextureIndex : register(DRAWCALLCONSTANTS)
{
    uint originTextureIndex;
}

static Texture2D<float4> originTexture = ResourceDescriptorHeap[originTextureIndex];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float4 color = originTexture.Sample(linearClampSampler, texCoord);
    
    color.a = dot(color.rgb, float3(0.299, 0.587, 0.114));
    
    return color;
}
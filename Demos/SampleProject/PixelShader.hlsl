#include"Common.hlsli"

//https://www.shadertoy.com/view/XlXcW4

float3 hash(uint3 x)
{
    x = ((x >> 8U) ^ x.yzx) * 1103515245U;
    x = ((x >> 8U) ^ x.yzx) * 1103515245U;
    x = ((x >> 8U) ^ x.yzx) * 1103515245U;
    
    return float3(x) * (1.0 / float(0xffffffffU));
}

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    uint3 coor = uint3(uint2(floor(texCoord * perframeResource.screenSize)), perframeResource.uintSeed);
    
    return float4(hash(coor), 1.0);
}
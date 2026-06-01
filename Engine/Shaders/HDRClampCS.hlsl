#include"Common.hlsli"

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint textureIndex;
}

static RWTexture2D<float4> hdrTexture = ResourceDescriptorHeap[textureIndex];

[numthreads(16, 16, 1)]
void main(const uint2 DTid : SV_DispatchThreadID )
{
    hdrTexture[DTid] = clamp(hdrTexture[DTid], 0.0, 65504.0);
}
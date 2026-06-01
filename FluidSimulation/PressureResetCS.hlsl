#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint pressureTexIndex;
}

static RWTexture2D<float> pressureTex = ResourceDescriptorHeap[pressureTexIndex];

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    pressureTex[DTid] = 0.0;
}
#include"Common.hlsli"

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint depthReadTexIndex;
    uint depthWriteTexIndex;
}

static Texture2D<float> depthRead = ResourceDescriptorHeap[depthReadTexIndex];

static RWTexture2D<float> depthWrite = ResourceDescriptorHeap[depthWriteTexIndex];

[numthreads(16, 16, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
    depthWrite[DTid] = depthRead[DTid];
}
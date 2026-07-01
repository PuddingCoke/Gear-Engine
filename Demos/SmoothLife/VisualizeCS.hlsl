#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint outputTextureIndex;
    uint readTextureIndex;
}

static RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[outputTextureIndex];

static Texture2D<float> readTexture = ResourceDescriptorHeap[readTextureIndex];

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    outputTexture[DTid] = float4(readTexture[DTid].rrr, 1.0);
}
#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint outputTextureIndex;
}

static RWTexture2D<float2> outputTexture = ResourceDescriptorHeap[outputTextureIndex];

[numthreads(8, 8, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float sign = ((((DTid.x + DTid.y) & 1) == 1) ? -1.0 : 1.0);
    
    const float2 result = outputTexture[DTid] * sign;
    
    outputTexture[DTid] = result;
}
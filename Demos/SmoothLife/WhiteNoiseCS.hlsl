#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint outputTextureIndex;
    uint uintSeed;
}

static RWTexture2D<float> outputTexture = ResourceDescriptorHeap[outputTextureIndex];

uint hash(uint2 x)
{
    const uint m = 0x5bd1e995U;
    uint hash = uintSeed;
    // process first vector element
    uint k = x.x;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;
    // process second vector element
    k = x.y;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;
	// some final mixing
    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;
    return hash;
}

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID )
{
    uint hashValue = hash(DTid);
    
    float value = float(hashValue) / float(0xffffffffU);
    
    outputTexture[DTid] = value * 0.75;
}
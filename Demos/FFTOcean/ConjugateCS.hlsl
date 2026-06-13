#include"Predefine.hlsli"

#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint waveSpectrumTextureIndex;
    uint tildeh0TextureIndex;
}

static RWTexture2D<float4> waveSpectrumTexture = ResourceDescriptorHeap[waveSpectrumTextureIndex];

static Texture2D<float2> tildeh0Texture = ResourceDescriptorHeap[tildeh0TextureIndex];

[numthreads(8, 8, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float2 tildeh0k = tildeh0Texture[DTid];
    
    const float2 tildeh0Minusk = tildeh0Texture[uint2((TEXTURESIZE - DTid.x) % TEXTURESIZE, (TEXTURESIZE - DTid.y) % TEXTURESIZE)];
    
    waveSpectrumTexture[DTid] = float4(tildeh0k.x, tildeh0k.y, tildeh0Minusk.x, -tildeh0Minusk.y);
}
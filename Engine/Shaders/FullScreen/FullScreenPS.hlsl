#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint textureIdx;
}

static Texture2D texture = ResourceDescriptorHeap[textureIdx];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    return texture.Sample(linearClampSampler, texCoord);
}
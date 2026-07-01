#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint arrowTextureIndex;
}

static Texture2D<float4> arrowTexture = ResourceDescriptorHeap[arrowTextureIndex];

struct PixelInput
{
    float2 texCoord : TEXCOORD;
};

float4 main(PixelInput input) : SV_TARGET
{
    return arrowTexture.Sample(linearClampSampler, input.texCoord);

}
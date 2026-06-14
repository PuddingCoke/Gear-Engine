#include"Common.hlsli"

struct PixelInput
{
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

cbuffer ResourceIndices : register(PER_INVOKE_CONSTANTS)
{
    uint fontTexIndex;
}

static Texture2D<float4> fontTex = ResourceDescriptorHeap[fontTexIndex];

float4 main(PixelInput input) : SV_TARGET
{
    float4 color = fontTex.Sample(linearWrapSampler, input.texCoord);
    
    return color * input.color;
}
#include"Common.hlsli"

struct PixelInput
{
    float3 position : POSITION;
};

struct PixelOutput
{
    float4 position : SV_Position;
};

PixelOutput main(PixelInput input)
{
    PixelOutput output;
    
    output.position = mul(float4(input.position, 1.0), perframeResource.viewProj);
    
    return output;
}
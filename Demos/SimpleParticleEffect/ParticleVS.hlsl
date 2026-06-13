#include"Common.hlsli"

struct VertexInput
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct VertexOutput
{
    float4 color : COLOR;
    float4 position : SV_POSITION;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.color = input.color;
    output.position = input.position;
    return output;
}
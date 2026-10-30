#include"Common.hlsli"

struct VertexInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct VertexOutput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float4 svPos : SV_POSITION;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.pos = input.pos;
    output.normal = input.normal;
    output.svPos = mul(float4(input.pos, 1.0), perframeResource.viewProj);
    return output;
}
#include"Common.hlsli"

struct GeometryInput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

struct GeometryOutput
{
    float4 color : COLOR;
    float4 pos : SV_Position;
};

cbuffer LineParam : register(DRAWCALLCONSTANTS)
{
    float lineWidth;
};

[maxvertexcount(4)]
void main(
	line GeometryInput input[2],
	inout TriangleStream<GeometryOutput> outputs
)
{
    const float2 dir = input[1].pos.xy - input[0].pos.xy;
    const float2 dirN = normalize(dir);
    const float2 v = lineWidth * float2(-dirN.y, dirN.x);
    
    GeometryOutput rect;
    
    rect.color = input[0].color;
    rect.pos = mul(input[0].pos - float4(v, 0.0, 0.0), perframeResource.proj);
    outputs.Append(rect);
    rect.pos = mul(input[0].pos + float4(v, 0.0, 0.0), perframeResource.proj);
    outputs.Append(rect);
    rect.pos = mul(input[1].pos - float4(v, 0.0, 0.0), perframeResource.proj);
    outputs.Append(rect);
    rect.pos = mul(input[1].pos + float4(v, 0.0, 0.0), perframeResource.proj);
    outputs.Append(rect);
}
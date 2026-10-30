#include"Common.hlsli"

struct GSInput
{
    float4 position : SV_Position;
    float2 size : SIZE;
    float4 color : COLOR;
    float uvLeft : TEXCOORD0;
    float uvRight : TEXCOORD1;
    float uvBottom : TEXCOORD2;
    float uvTop : TEXCOORD3;
};

struct GSOutput
{
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    float4 pos : SV_POSITION;
};

[maxvertexcount(4)]
void main(
	point GSInput input[1],
	inout TriangleStream<GSOutput> outputs
)
{
    GSOutput output;
    output.color = input[0].color;
    
    float3 cameraRight = perframeResource.cameraRight.xyz;
    float3 cameraUp = perframeResource.cameraUp.xyz;
    
    output.pos = mul(input[0].position + float4(cameraRight * -0.5 * input[0].size.x + cameraUp * -0.5 * input[0].size.y, 0.0), perframeResource.viewProj);
    output.texCoord = float2(input[0].uvLeft, input[0].uvBottom);
    outputs.Append(output);
    
    output.pos = mul(input[0].position + float4(cameraRight * -0.5 * input[0].size.x + cameraUp * 0.5 * input[0].size.y, 0.0), perframeResource.viewProj);
    output.texCoord = float2(input[0].uvLeft, input[0].uvTop);
    outputs.Append(output);
    
    output.pos = mul(input[0].position + float4(cameraRight * 0.5 * input[0].size.x + cameraUp * -0.5 * input[0].size.y, 0.0), perframeResource.viewProj);
    output.texCoord = float2(input[0].uvRight, input[0].uvBottom);
    outputs.Append(output);
    
    output.pos = mul(input[0].position + float4(cameraRight * 0.5 * input[0].size.x + cameraUp * 0.5 * input[0].size.y, 0.0), perframeResource.viewProj);
    output.texCoord = float2(input[0].uvRight, input[0].uvTop);
    outputs.Append(output);
}

#include"Common.hlsli"

struct GeometryInput
{
    float4 color : COLOR;
    float4 position : SV_POSITION;
};

struct GeometryOutput
{
    float4 color : COLOR;
    float4 position : SV_POSITION;
};

cbuffer SimulationParam : register(PER_INVOKE_CONSTANTS)
{
    float dissipativeFactor;
    int simulationSteps;
}

#define MAXVERTEXCOUNT 16

void GetNextPosition(inout float4 pos)
{
    [unroll]
    for (int i = 0; i < 10; i++)
    {
        const float dx = (sin(pos.y) - dissipativeFactor * pos.x) * perframeResource.deltaTime;
        const float dy = (sin(pos.z) - dissipativeFactor * pos.y) * perframeResource.deltaTime;
        const float dz = (sin(pos.x) - dissipativeFactor * pos.z) * perframeResource.deltaTime;

        pos += float4(dx, dy, dz, 0.0);
    }
}

[maxvertexcount(MAXVERTEXCOUNT)]
void main(
	point GeometryInput inputs[1],
	inout LineStream<GeometryOutput> outputs
)
{
    GeometryOutput output;
    
    float4 pos = inputs[0].position;
    
    output.color = inputs[0].color;
    
    [unroll]
    for (uint i = 0; i < MAXVERTEXCOUNT; i++)
    {
        output.position = mul(pos, perframeResource.viewProj);
        
        outputs.Append(output);
        
        GetNextPosition(pos);
    }
    
    outputs.RestartStrip();
}
//编译测试utf-8
#include"Common.hlsli"

cbuffer SimulationParam : register(DRAWCALLCONSTANTS)
{
    uint positionBufferIndex;
    float dissipativeFactor;
    int simulationSteps;
}

static RWBuffer<float4> positions = ResourceDescriptorHeap[positionBufferIndex];

[numthreads(100, 1, 1)]
void main(uint DTid : SV_DispatchThreadID)
{
    float4 pos = positions[DTid];
    
    for (int i = 0; i < simulationSteps; i++)
    {
        const float dx = (sin(pos.y) - dissipativeFactor * pos.x) * perframeResource.deltaTime;
        const float dy = (sin(pos.z) - dissipativeFactor * pos.y) * perframeResource.deltaTime;
        const float dz = (sin(pos.x) - dissipativeFactor * pos.z) * perframeResource.deltaTime;

        pos += float4(dx, dy, dz, 0.0);
    }
    
    positions[DTid] = pos;
}
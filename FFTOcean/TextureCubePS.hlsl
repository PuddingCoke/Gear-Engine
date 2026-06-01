#include"Common.hlsli"

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint enviromentCubeIndex;
}

static TextureCube<float4> enviromentCube = ResourceDescriptorHeap[enviromentCubeIndex];

float4 main(const float3 position : POSITION) : SV_TARGET
{  
    const float3 color = enviromentCube.Sample(linearWrapSampler, normalize(position)).rgb;
    
    return float4(color, 1.0);
}
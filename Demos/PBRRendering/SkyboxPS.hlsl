#include"Common.hlsli"

cbuffer TextureIndex : register(PER_INVOKE_CONSTANTS)
{
    uint envCubeIndex;
}

float4 main(float3 position : POSITION) : SV_TARGET
{
    TextureCube<float4> envCube = ResourceDescriptorHeap[envCubeIndex];
    float3 envColor = envCube.Sample(linearClampSampler, position).rgb;
    return float4(envColor, 1.0);
}
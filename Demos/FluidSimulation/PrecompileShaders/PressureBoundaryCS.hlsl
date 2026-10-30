#include"Common.hlsli"
#include"Utility.hlsli"

cbuffer SimulationParam : register(USER_GLOBAL_CBUFFER)
{
    float2 pos;
    float2 posDelta;
    float4 splatColor;
    float2 colorTexelSize;
    float2 simTexelSize;
    uint2 colorTextureSize;
    uint2 simTextureSize;
    float colorDissipationSpeed;
    float velocityDissipationSpeed;
    float vorticityIntensity;
    float splatRadius;
}

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint pressureReadTexIndex;
    uint pressureWriteTexIndex;
}

static Texture2D<float> pressureReadTex = ResourceDescriptorHeap[pressureReadTexIndex];

static RWTexture2D<float> pressureWriteTex = ResourceDescriptorHeap[pressureWriteTexIndex];

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float2 position = float2(DTid) + float2(0.5, 0.5);
    
    const float2 dir = position - obstaclePosition;
    
    const float len = length(dir);
    
    if (len < obstacleRadius)
    {
        pressureWriteTex[DTid] = 0.0;
    }
    else if (len < obstacleRadius + 1.0)
    {
        pressureWriteTex[DTid] = pressureReadTex.SampleLevel(linearClampSampler, (position + normalize(dir)) * simTexelSize, 0.0);
    }
    else
    {
        pressureWriteTex[DTid] = pressureReadTex[DTid];
    }
}
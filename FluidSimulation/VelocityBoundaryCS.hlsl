#include"Common.hlsli"
#include"Utility.hlsli"

cbuffer SimulationParam : register(USERGLOBALCBUFFER)
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

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint velocityReadTexIndex;
    uint velocityWriteTexIndex;
}

static Texture2D<float2> velocityReadTex = ResourceDescriptorHeap[velocityReadTexIndex];

static RWTexture2D<float2> velocityWriteTex = ResourceDescriptorHeap[velocityWriteTexIndex];

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float2 position = float2(DTid) + float2(0.5, 0.5);
    
    const float2 dir = position - obstaclePosition;
    
    const float len = length(dir);
    
    if (len < obstacleRadius)
    {
        velocityWriteTex[DTid] = float2(0.0, 0.0);
    }
    else if (len < obstacleRadius + 1.0)
    {
        velocityWriteTex[DTid] = -velocityReadTex.SampleLevel(linearClampSampler, (position + normalize(dir)) * simTexelSize, 0.0);
    }
    else
    {
        velocityWriteTex[DTid] = velocityReadTex[DTid];
    }
}
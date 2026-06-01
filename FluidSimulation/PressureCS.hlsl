#include"Common.hlsli"

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
    uint divergenceTexIndex;
    uint pressureReadTexIndex;
    uint pressureWriteTexIndex;
};

static Texture2D<float> divergenceTex = ResourceDescriptorHeap[divergenceTexIndex];

static Texture2D<float> pressureReadTex = ResourceDescriptorHeap[pressureReadTexIndex];

static RWTexture2D<float> pressureWriteTex = ResourceDescriptorHeap[pressureWriteTexIndex];

float PressureAt(const uint2 loc)
{
    const float R = pressureReadTex[loc + uint2(1, 0)];
        
    const float L = pressureReadTex[loc - uint2(1, 0)];
        
    const float T = pressureReadTex[loc + uint2(0, 1)];
        
    const float B = pressureReadTex[loc - uint2(0, 1)];
        
    const float C = divergenceTex[loc];
    
    const float pressure = (R + L + T + B - C) * 0.25;
    
    return pressure;
}

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    //interior texel
    if (DTid.x > 0 && DTid.x < simTextureSize.x - 1 && DTid.y > 0 && DTid.y < simTextureSize.y - 1)
    {
        pressureWriteTex[DTid] = PressureAt(DTid);
    }
    //row texel
    else if (DTid.x > 0 && DTid.x < simTextureSize.x - 1)
    {
        if (DTid.y == 0)
        {
            pressureWriteTex[DTid] = PressureAt(uint2(DTid.x, 1));
        }
        else if (DTid.y == simTextureSize.y - 1)
        {
            pressureWriteTex[DTid] = PressureAt(uint2(DTid.x, simTextureSize.y - 2));
        }
    }
    //column texel
    else if (DTid.y > 0 && DTid.y < simTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            pressureWriteTex[DTid] = PressureAt(uint2(1, DTid.y));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            pressureWriteTex[DTid] = PressureAt(uint2(simTextureSize.x - 2, DTid.y));
        }
    }
    //corner texel
    else if (DTid.y == 0)
    {
        if (DTid.x == 0)
        {
            pressureWriteTex[DTid] = PressureAt(uint2(1, 1));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            pressureWriteTex[DTid] = PressureAt(uint2(simTextureSize.x - 2, 1));
        }
    }
    else if (DTid.y == simTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            pressureWriteTex[DTid] = PressureAt(uint2(1, simTextureSize.y - 2));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            pressureWriteTex[DTid] = PressureAt(uint2(simTextureSize.x - 2, simTextureSize.y - 2));
        }
    }
}
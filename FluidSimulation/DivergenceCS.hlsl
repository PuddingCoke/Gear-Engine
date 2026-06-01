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
    uint velocityTexIndex;
    uint divergenceTexIndex;
};

static Texture2D<float2> velocityTex = ResourceDescriptorHeap[velocityTexIndex];

static RWTexture2D<float> divergenceTex = ResourceDescriptorHeap[divergenceTexIndex];

float DivergenceAt(const uint2 loc)
{
    const float uR = velocityTex[loc + uint2(1, 0)].x;
        
    const float uL = velocityTex[loc - uint2(1, 0)].x;
        
    const float vT = velocityTex[loc + uint2(0, 1)].y;
        
    const float vB = velocityTex[loc - uint2(0, 1)].y;
        
    const float divergence = 0.5 * (uR - uL + vT - vB);
    
    return divergence;
}

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    //interior texel
    if (DTid.x > 0 && DTid.x < simTextureSize.x - 1 && DTid.y > 0 && DTid.y < simTextureSize.y - 1)
    {
        divergenceTex[DTid] = DivergenceAt(DTid);
    }
    //row texel
    else if (DTid.x > 0 && DTid.x < simTextureSize.x - 1)
    {
        if (DTid.y == 0)
        {
            divergenceTex[DTid] = DivergenceAt(uint2(DTid.x, 1));
        }
        else if (DTid.y == simTextureSize.y - 1)
        {
            divergenceTex[DTid] = DivergenceAt(uint2(DTid.x, simTextureSize.y - 2));
        }
    }
    //column texel
    else if (DTid.y > 0 && DTid.y < simTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            divergenceTex[DTid] = DivergenceAt(uint2(1, DTid.y));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            divergenceTex[DTid] = DivergenceAt(uint2(simTextureSize.x - 2, DTid.y));
        }
    }
    //corner texel
    else if (DTid.y == 0)
    {
        if (DTid.x == 0)
        {
            divergenceTex[DTid] = DivergenceAt(uint2(1, 1));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            divergenceTex[DTid] = DivergenceAt(uint2(simTextureSize.x - 2, 1));
        }
    }
    else if (DTid.y == simTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            divergenceTex[DTid] = DivergenceAt(uint2(1, simTextureSize.y - 2));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            divergenceTex[DTid] = DivergenceAt(uint2(simTextureSize.x - 2, simTextureSize.y - 2));
        }
    }
}
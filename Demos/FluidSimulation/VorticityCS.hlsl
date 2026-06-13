#include"Common.hlsli"

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
    uint velocityTexIndex;
    uint vorticityTexIndex;
}

static Texture2D<float2> velocityTex = ResourceDescriptorHeap[velocityTexIndex];

static RWTexture2D<float> vorticityTex = ResourceDescriptorHeap[vorticityTexIndex];

float VorticityAt(const uint2 loc)
{
    const float vR = velocityTex[loc + uint2(1, 0)].y;
    
    const float vL = velocityTex[loc - uint2(1, 0)].y;
    
    const float uT = velocityTex[loc + uint2(0, 1)].x;
    
    const float uB = velocityTex[loc - uint2(0, 1)].x;
    
    const float vorticity = 0.5 * (uT - uB + vL - vR);
    
    return vorticity;
}

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    //interior texel
    if (DTid.x > 0 && DTid.x < simTextureSize.x - 1 && DTid.y > 0 && DTid.y < simTextureSize.y - 1)
    {
        vorticityTex[DTid] = VorticityAt(DTid);
    }
    //row texel
    else if (DTid.x > 0 && DTid.x < simTextureSize.x - 1)
    {
        if (DTid.y == 0)
        {
            vorticityTex[DTid] = VorticityAt(uint2(DTid.x, 1));
        }
        else if (DTid.y == simTextureSize.y - 1)
        {
            vorticityTex[DTid] = VorticityAt(uint2(DTid.x, simTextureSize.y - 2));
        }
    }
    //column texel
    else if (DTid.y > 0 && DTid.y < simTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            vorticityTex[DTid] = VorticityAt(uint2(1, DTid.y));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            vorticityTex[DTid] = VorticityAt(uint2(simTextureSize.x - 2, DTid.y));
        }
    }
    //corner texel
    else if (DTid.y == 0)
    {
        if (DTid.x == 0)
        {
            vorticityTex[DTid] = VorticityAt(uint2(1, 1));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            vorticityTex[DTid] = VorticityAt(uint2(simTextureSize.x - 2, 1));
        }
    }
    else if (DTid.y == simTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            vorticityTex[DTid] = VorticityAt(uint2(1, simTextureSize.y - 2));
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            vorticityTex[DTid] = VorticityAt(uint2(simTextureSize.x - 2, simTextureSize.y - 2));
        }
    }
}
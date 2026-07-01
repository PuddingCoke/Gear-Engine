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
    uint pressureTexIndex;
    uint velocityReadTexIndex;
    uint velocityWriteTexIndex;
};

static Texture2D<float> pressureTex = ResourceDescriptorHeap[pressureTexIndex];

static Texture2D<float2> velocityReadTex = ResourceDescriptorHeap[velocityReadTexIndex];

static RWTexture2D<float2> velocityWriteTex = ResourceDescriptorHeap[velocityWriteTexIndex];

float2 VelocityAt(const uint2 loc)
{
    const float R = pressureTex[loc + uint2(1, 0)];
    
    const float L = pressureTex[loc - uint2(1, 0)];
    
    const float T = pressureTex[loc + uint2(0, 1)];
    
    const float B = pressureTex[loc - uint2(0, 1)];
    
    const float2 velocity = velocityReadTex[loc] - 0.5 * float2(R - L, T - B);
    
    return velocity;
}

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    //interior texel
    if (DTid.x > 0 && DTid.x < simTextureSize.x - 1 && DTid.y > 0 && DTid.y < simTextureSize.y - 1)
    {
        velocityWriteTex[DTid] = VelocityAt(DTid);
    }
    //row texel
    else if (DTid.x > 0 && DTid.x < simTextureSize.x - 1)
    {
        if (DTid.y == 0)
        {
            float2 velocity = VelocityAt(uint2(DTid.x, 1));
            
            velocity.y = -velocity.y;
            
            velocityWriteTex[DTid] = velocity;
        }
        else if (DTid.y == simTextureSize.y - 1)
        {
            float2 velocity = VelocityAt(uint2(DTid.x, simTextureSize.y - 2));
             
            velocity.y = -velocity.y;
                
            velocityWriteTex[DTid] = velocity;
        }
    }
    //column texel
    else if (DTid.y > 0 && DTid.y < simTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            float2 velocity = VelocityAt(uint2(1, DTid.y));
            
            velocity.x = -velocity.x;
            
            velocityWriteTex[DTid] = velocity;
        }
        else if (DTid.x == simTextureSize.x - 1)
        {
            float2 velocity = VelocityAt(uint2(simTextureSize.x - 2, DTid.y));
            
            velocity.x = -velocity.x;
            
            velocityWriteTex[DTid] = velocity;
        }
    }
    //corner texel
    else if ((DTid.x == 0 || DTid.x == simTextureSize.x - 1) && (DTid.y == 0 || DTid.y == simTextureSize.y - 1))
    {
        velocityWriteTex[DTid] = float2(0.0, 0.0);
    }
}
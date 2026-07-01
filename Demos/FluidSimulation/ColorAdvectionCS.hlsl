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
    uint colorReadTexIndex;
    uint colorWriteTexIndex;
}

static Texture2D<float2> velocityTex = ResourceDescriptorHeap[velocityTexIndex];

static Texture2D<float4> colorReadTex = ResourceDescriptorHeap[colorReadTexIndex];

static RWTexture2D<float4> colorWriteTex = ResourceDescriptorHeap[colorWriteTexIndex];

float4 ColorAt(const uint2 loc)
{
    float2 texCoord = (float2(loc) + float2(0.5, 0.5)) * colorTexelSize;
    
    texCoord -= perframeResource.deltaTime * velocityTex.SampleLevel(linearClampSampler, texCoord, 0.0) * simTexelSize;
    
    const float3 color = colorReadTex.SampleLevel(linearClampSampler, texCoord, 0.0).rgb;
    
    const float dissipation = 1.0 + colorDissipationSpeed * perframeResource.deltaTime;
    
    return float4(color / dissipation, 1.0);
}

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{ 
    //interior texel
    if (DTid.x > 0 && DTid.x < colorTextureSize.x - 1 && DTid.y > 0 && DTid.y < colorTextureSize.y - 1)
    {
        colorWriteTex[DTid] = ColorAt(DTid);
    }
    //row texel
    else if (DTid.x > 0 && DTid.x < colorTextureSize.x - 1)
    {
        if (DTid.y == 0)
        {
            colorWriteTex[DTid] = ColorAt(uint2(DTid.x, 1));
        }
        else if (DTid.y == colorTextureSize.y - 1)
        {
            colorWriteTex[DTid] = ColorAt(uint2(DTid.x, colorTextureSize.y - 2));
        }
    }
    //column texel
    else if (DTid.y > 0 && DTid.y < colorTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            colorWriteTex[DTid] = ColorAt(uint2(1, DTid.y));
        }
        else if (DTid.x == colorTextureSize.x - 1)
        {
            colorWriteTex[DTid] = ColorAt(uint2(colorTextureSize.x - 2, DTid.y));
        }
    }
    //corner texel
    else if (DTid.y == 0)
    {
        if (DTid.x == 0)
        {
            colorWriteTex[DTid] = ColorAt(uint2(1, 1));
        }
        else if (DTid.x == colorTextureSize.x - 1)
        {
            colorWriteTex[DTid] = ColorAt(uint2(colorTextureSize.x - 2, 1));
        }
    }
    else if (DTid.y == colorTextureSize.y - 1)
    {
        if (DTid.x == 0)
        {
            colorWriteTex[DTid] = ColorAt(uint2(1, colorTextureSize.y - 2));
        }
        else if (DTid.x == colorTextureSize.x - 1)
        {
            colorWriteTex[DTid] = ColorAt(uint2(colorTextureSize.x - 2, colorTextureSize.y - 2));
        }
    }
}
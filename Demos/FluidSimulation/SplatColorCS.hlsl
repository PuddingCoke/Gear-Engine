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
    uint colorReadTexIndex;
    uint colorWriteTexIndex;
}

static Texture2D<float4> colorReadTex = ResourceDescriptorHeap[colorReadTexIndex];

static RWTexture2D<float4> colorWriteTex = ResourceDescriptorHeap[colorWriteTexIndex];

float4 ColorAt(const uint2 loc)
{
    float2 relativePos = float2(loc) + float2(0.5, 0.5);
    
    relativePos -= pos;
    
    const float3 color = exp(-dot(relativePos, relativePos) / (splatRadius * float(colorTextureSize.y) * float(colorTextureSize.y))) * splatColor.rgb * 0.15;
        
    const float3 curColor = colorReadTex[loc].rgb;
    
    return float4(color + curColor, 1.0);
}

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float scale = simTexelSize.x / colorTexelSize.x;
    
    const float scaledRadius = obstacleRadius * scale;
    
    const float2 scaledPosition = obstaclePosition * scale;
    
    if (distance(float2(DTid) + float2(0.5, 0.5), scaledPosition) < scaledRadius)
    {
        colorWriteTex[DTid] = float4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        colorWriteTex[DTid] = ColorAt(DTid);
    }
}
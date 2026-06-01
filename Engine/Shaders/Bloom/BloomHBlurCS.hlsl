#include"Common.hlsli"

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint sourceTextureIdx;
    uint destTextureIdx;
}

cbuffer BlurParam : register(DRAWCALLCBUFFER)
{
    float4 weights[2];
    float4 offsets[2];
    float2 texelSize;
    uint iteration;
    float sigma;
}

static Texture2D<float4> sourceTexture = ResourceDescriptorHeap[sourceTextureIdx];

static RWTexture2D<float4> destTexture = ResourceDescriptorHeap[destTextureIdx];

static const float1 weight[8] = (float1[8]) weights;

static const float1 offset[8] = (float1[8]) offsets;

[numthreads(60, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    const float2 texCoord = (float2(DTid.xy) + 0.5) * texelSize;
    
    float3 result = sourceTexture.SampleLevel(linearClampSampler, texCoord, 0.0).rgb * weight[0].x;
    
    for (uint i = 1; i < iteration; ++i)
    {
        result += sourceTexture.SampleLevel(linearClampSampler, texCoord + float2(texelSize.x * offset[i].x, 0.0), 0.0).rgb * weight[i].x;
        result += sourceTexture.SampleLevel(linearClampSampler, texCoord - float2(texelSize.x * offset[i].x, 0.0), 0.0).rgb * weight[i].x;
    }
    
    destTexture[DTid.xy] = float4(result, 1.0);
}
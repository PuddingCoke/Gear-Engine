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
    float kA;
    float kD;
    float bumpScale;
    float edgeMagnitudeScale;
}

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint colorTexIndex;
    uint edgeHighlightTexIndex;
}

static Texture2D<float4> colorTex = ResourceDescriptorHeap[colorTexIndex];

static RWTexture2D<float4> edgeHighlightTex = ResourceDescriptorHeap[edgeHighlightTexIndex];

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    if (DTid.x > 0 && DTid.x < colorTextureSize.x - 1 && DTid.y > 0 && DTid.y < colorTextureSize.y - 1)
    {
        const float3 center = colorTex[DTid].rgb;
    
        const float3 right = colorTex[DTid + uint2(1, 0)].rgb;

        const float3 left = colorTex[DTid - uint2(1, 0)].rgb;

        const float3 top = colorTex[DTid + uint2(0, 1)].rgb;

        const float3 bottom = colorTex[DTid - uint2(0, 1)].rgb;

        const float leftLuma = dot(left, float3(0.299, 0.587, 0.114));

        const float rightLuma = dot(right, float3(0.299, 0.587, 0.114));

        const float topLuma = dot(top, float3(0.299, 0.587, 0.114));

        const float bottomLuma = dot(bottom, float3(0.299, 0.587, 0.114));
   
        const float dx = (rightLuma - leftLuma) * 0.5;

        const float dy = (topLuma - bottomLuma) * 0.5;

        const float edgeMagnitude = length(float2(dx, dy)) * edgeMagnitudeScale;

        const float3 color = center * (1.0 + edgeMagnitude);
        
        edgeHighlightTex[DTid] = float4(color, 1.0);
    }
    else
    {
        edgeHighlightTex[DTid] = float4(0.0, 0.0, 0.0, 1.0);
    }
}
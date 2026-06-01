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
    float kA;
    float kD;
    float bumpScale;
}

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint colorTexIndex;
    uint diffuseTexIndex;
}

static Texture2D<float4> colorTex = ResourceDescriptorHeap[colorTexIndex];

static RWTexture2D<float4> diffuseTex = ResourceDescriptorHeap[diffuseTexIndex];

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

        const float leftHeight = dot(left, float3(0.299, 0.587, 0.114));

        const float rightHeight = dot(right, float3(0.299, 0.587, 0.114));

        const float topHeight = dot(top, float3(0.299, 0.587, 0.114));

        const float bottomHeight = dot(bottom, float3(0.299, 0.587, 0.114));
   
        const float dx = (rightHeight - leftHeight) * 0.5;

        const float dy = (topHeight - bottomHeight) * 0.5;
        
        const float3 normal = normalize(float3(-dx, -dy, bumpScale));
    
        const float3 light = float3(0.0, 0.0, 1.0);
    
        const float3 ambient = kA * center;

        const float3 diffuse = kD * center * saturate(dot(normal, light));

        const float3 color = ambient + diffuse;

        diffuseTex[DTid] = float4(color, 1.0);
    }
    else
    {
        diffuseTex[DTid] = float4(0.0, 0.0, 0.0, 1.0);
    }
}
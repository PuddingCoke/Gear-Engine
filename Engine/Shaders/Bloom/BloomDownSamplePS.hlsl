#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint sourceTextureIdx;
};

static Texture2D<float4> sourceTexture = ResourceDescriptorHeap[sourceTextureIdx];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float2 texelSize;
    sourceTexture.GetDimensions(texelSize.x, texelSize.y);
    texelSize = 1.0 / texelSize;
    
    const float3 a = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - 2 * texelSize.x, texCoord.y + 2 * texelSize.y)).rgb;
    const float3 b = sourceTexture.Sample(linearClampSampler, float2(texCoord.x, texCoord.y + 2 * texelSize.y)).rgb;
    const float3 c = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + 2 * texelSize.x, texCoord.y + 2 * texelSize.y)).rgb;

    const float3 d = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - 2 * texelSize.x, texCoord.y)).rgb;
    const float3 e = sourceTexture.Sample(linearClampSampler, float2(texCoord.x, texCoord.y)).rgb;
    const float3 f = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + 2 * texelSize.x, texCoord.y)).rgb;

    const float3 g = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - 2 * texelSize.x, texCoord.y - 2 * texelSize.y)).rgb;
    const float3 h = sourceTexture.Sample(linearClampSampler, float2(texCoord.x, texCoord.y - 2 * texelSize.y)).rgb;
    const float3 i = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + 2 * texelSize.x, texCoord.y - 2 * texelSize.y)).rgb;

    const float3 j = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - texelSize.x, texCoord.y + texelSize.y)).rgb;
    const float3 k = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + texelSize.x, texCoord.y + texelSize.y)).rgb;
    const float3 l = sourceTexture.Sample(linearClampSampler, float2(texCoord.x - texelSize.x, texCoord.y - texelSize.y)).rgb;
    const float3 m = sourceTexture.Sample(linearClampSampler, float2(texCoord.x + texelSize.x, texCoord.y - texelSize.y)).rgb;
    
    float3 color = e * 0.125;
    color += (a + c + g + i) * 0.03125;
    color += (b + d + f + h) * 0.0625;
    color += (j + k + l + m) * 0.125;
    
    return float4(color, 1.0);
}
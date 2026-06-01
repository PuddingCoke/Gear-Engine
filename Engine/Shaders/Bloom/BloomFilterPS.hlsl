#include"Common.hlsli"

cbuffer BloomParam : register(PER_INVOKE_CONSTANTS)
{
    uint originTextureIdx;
    float exposure;
    float gamma;
    float threshold;
    float intensity;
    float softThreshold;
};

static Texture2D<float4> originTexture = ResourceDescriptorHeap[originTextureIdx];

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    const float3 color = max(originTexture.Sample(linearClampSampler, texCoord).rgb, float3(0.0, 0.0, 0.0));
    
    float brightness = max(color.r, max(color.g, color.b));
    
    float knee = threshold * softThreshold;
    
    float soft = brightness - threshold + knee;
    
    soft = clamp(soft, 0.0, 2.0 * knee);
    
    soft = soft * soft / (4.0 * knee + 0.00001);
    
    float contribution = max(soft, brightness - threshold);
    
    contribution /= max(brightness, 0.00001);
    
    return float4(color * contribution, 1.0);
}
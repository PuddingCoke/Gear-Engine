#include"Common.hlsli"

struct PixelInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct PixelOutput
{
    float4 positionMetallic : SV_Target0;
    float4 normalRoughness : SV_Target1;
    float4 baseColor : SV_Target2;
};

cbuffer TextureIndex : register(b2)
{
    uint diffuseTexIndex;
    uint roughnessMetallicTexIndex;
    uint normalTexIndex;
    float clipMaxDistance;
    float clipExponent;
}

static Texture2D tDiffuse = ResourceDescriptorHeap[diffuseTexIndex];

static Texture2D tRoughnessMetallic = ResourceDescriptorHeap[roughnessMetallicTexIndex];

static Texture2D tNormal = ResourceDescriptorHeap[normalTexIndex];

float ToksvigEquation(float alphaG, float averageNormalLength)
{
    float alphaP = 2.0 / (alphaG * alphaG) - 2.0;

    float alphaPPrime = averageNormalLength * alphaP / max((averageNormalLength + alphaP * (1.0 - averageNormalLength)), 1e-6);
    
    float alphaGPrime = sqrt(2.0 / (alphaPPrime + 2.0));
    
    return alphaGPrime;
}

PixelOutput main(PixelInput input)
{
    float4 baseColor = tDiffuse.Sample(anisotrophicWrapSampler, input.uv);
    
    float dist = distance(perframeResource.eyePos.xyz, input.pos.xyz);
    
    float alphaBlendFactor = 1.0 - pow(dist / clipMaxDistance, clipExponent);
    
    clip(baseColor.a - alphaBlendFactor);
    
    float3 N = normalize(input.normal);
    float3 B = normalize(input.binormal);
    float3 T = normalize(input.tangent);
    float3x3 TBN = float3x3(T, B, N);
    
    float3 normal = tNormal.Sample(anisotrophicWrapSampler, input.uv).xyz * 2.0 - 1.0;
    
    float2 roughnessMetallic = tRoughnessMetallic.Sample(anisotrophicWrapSampler, input.uv).gb;
    
    float roughness = max(roughnessMetallic.r, 0.001);
    
    roughness = sqrt(ToksvigEquation(roughness * roughness, length(normal)));
    
    float metallic = roughnessMetallic.g;
    
    PixelOutput output;
    output.positionMetallic = float4(input.pos, metallic);
    output.normalRoughness = float4(mul(normalize(normal), TBN), roughness);
    output.baseColor = baseColor;
    
    return output;
}
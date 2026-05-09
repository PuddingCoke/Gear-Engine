#include"Common.hlsli"

#define PI 3.14159265358979323846

float GGXDistribution(float3 M, float3 N, float roughness)
{
    float alphaSqr = roughness * roughness;
    
    alphaSqr *= alphaSqr;
    
    float MdotN = max(dot(M, N), 0.0);
    
    float denominator = MdotN * MdotN * (alphaSqr - 1.0) + 1.0;
    
    denominator = PI * denominator * denominator;
    
    return alphaSqr / denominator;
}

float SmithG_1(float NdotV, float k)
{
    return NdotV / lerp(NdotV, 1.0, k);
}

float SmithG_2(float3 N, float3 L, float3 V, float k)
{
    const float NdotL = max(dot(N, L), 0.0);
    
    const float NdotV = max(dot(N, V), 0.0);
    
    return SmithG_1(NdotL, k) * SmithG_1(NdotV, k);
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

//analytic light source
float3 SpecularBRDF(float3 H, float3 N, float3 V, float3 L, float3 F0, float roughness)
{
    const float NdotV = max(dot(N, V), 0.0);
    
    const float NdotL = max(dot(N, L), 0.0);
    
    //analytic light source
    const float k = (roughness + 1) * (roughness + 1) / 8.0;
    
    const float3 numerator = GGXDistribution(H, N, roughness) * FresnelSchlick(saturate(dot(H, V)), F0) * SmithG_2(N, L, V, k);
    
    const float denominator = 4.0 * NdotL * NdotV + 0.00001;
    
    return numerator / denominator;
}

struct PixelInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

cbuffer SceneInfo : register(b2)
{
    float4 lightPos;
    float4 lightColor;
    float metallic;
    float roughness;
}

cbuffer Material : register(b3)
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specularColor;
}

float4 main(PixelInput input) : SV_TARGET
{
    float3 linearColor = pow(diffuseColor.rgb, 2.2);
    
    float3 N = normalize(input.normal);
    
    float3 L = normalize(lightPos.xyz - input.pos.xyz);
    
    float3 V = normalize(perframeResource.eyePos.xyz - input.pos.xyz);
    
    float3 H = normalize(L + V);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    
    float3 albedo = lerp(linearColor, float3(0.0, 0.0, 0.0), metallic);
    
    F0 = lerp(F0, linearColor, metallic);
    
    float3 radiance = lightColor / pow(length(input.pos - lightPos.xyz), 2.0);
    
    float NdotL = saturate(dot(N, L));
    
    float3 color = (albedo / PI + SpecularBRDF(H, N, V, L, F0, roughness)) * radiance * NdotL;
    
    return float4(color, 1.0f);
}
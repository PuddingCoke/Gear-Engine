#include"Common.hlsli"

#include"PBRHeader.hlsli"

struct PixelInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

cbuffer SceneInfo : register(PER_INVOKE_CONSTANTS)
{
    float4 lightPos;
    float4 lightColor;
    float metallic;
    float roughness;
    uint enableSpecularIBL;
    uint enableDiffuseIBL;
    uint prefilterMapIndex;
    uint brdfLUTTextureIndex;
    uint irradianceTextureIndex;
}

cbuffer Material : register(PER_INVOKE_CBUFFER)
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specularColor;
}

static TextureCube prefilterMap = ResourceDescriptorHeap[prefilterMapIndex];

static Texture2D<float2> brdfLUTTexture = ResourceDescriptorHeap[brdfLUTTextureIndex];

static TextureCube irradianceMap = ResourceDescriptorHeap[irradianceTextureIndex];

#define PI 3.141592653589793

float4 main(PixelInput input) : SV_TARGET
{
    float3 linearColor = pow(diffuseColor.rgb, 2.2);
    
    float3 N = normalize(input.normal);
    
    float3 L = normalize(lightPos.xyz - input.pos.xyz);
    
    float3 V = normalize(perframeResource.eyePos.xyz - input.pos.xyz);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    
    float3 albedo = lerp(linearColor, float3(0.0, 0.0, 0.0), metallic);
    
    F0 = lerp(F0, linearColor, metallic);
    
    float3 radiance = lightColor.rgb / pow(length(input.pos - lightPos.xyz), 2.0);
    
    float NdotL = saturate(dot(N, L));
    
    float3 color = PBR_BRDFEvaluate(N, V, L, F0, albedo, roughness) * radiance * NdotL;
    
    if (enableSpecularIBL)
    {
        float NdotV = saturate(dot(N, V));
    
        float3 R = 2.0 * dot(V, N) * N - V;
    
        const float miplevels = 6.0;
    
        float sampleLOD = roughness * (miplevels - 1.0);
    
        float3 prefilteredColor = prefilterMap.SampleLevel(linearWrapSampler, R, sampleLOD).rgb;
    
        float2 brdf = brdfLUTTexture.Sample(linearWrapSampler, float2(NdotV, roughness));
    
        color += prefilteredColor * (F0 * brdf.x + brdf.y);
    }
     
    if (enableDiffuseIBL)
    {
        float3 irradiance = irradianceMap.Sample(linearWrapSampler, N).rgb;
    
        color += albedo / PI * irradiance;
    }
        
    return float4(color, 1.0f);
}
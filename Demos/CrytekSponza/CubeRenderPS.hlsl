#include"Common.hlsli"

#include"PBRHeader.hlsli"

#include"Utility.hlsli"

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
    float4 color : SV_Target0;
    float dist : SV_Target1;
};

cbuffer TextureIndex : register(PER_INVOKE_CONSTANTS)
{
    uint diffuseTexIndex;
    uint roughnessMetallicTexIndex;
    uint normalTexIndex;
    uint shadowTexIndex;
    uint irradianceVolumeBufIndex;
};

cbuffer ProjMatrices : register(PER_INVOKE_CBUFFER)
{
    matrix viewProj[6];
    float3 probeLocation;
    uint probeIndex;
}

static Texture2D tDiffuse = ResourceDescriptorHeap[diffuseTexIndex];

static Texture2D tRoughnessMetallic = ResourceDescriptorHeap[roughnessMetallicTexIndex];

static Texture2D tNormal = ResourceDescriptorHeap[normalTexIndex];

static Texture2D<float> shadowTexture = ResourceDescriptorHeap[shadowTexIndex];

static ConstantBuffer<IrradianceVolume> volume = ResourceDescriptorHeap[irradianceVolumeBufIndex];

float CalShadow(float3 P)
{
    float4 shadowPos = mul(float4(P, 1.0), volume.lightViewProj);
    
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;
    
    return 1.0 - shadowTexture.SampleCmpLevelZero(shadowSampler, shadowPos.xy, shadowPos.z);
}

PixelOutput main(PixelInput input)
{
    const float4 baseColor = tDiffuse.Sample(linearWrapSampler, input.uv);
    
    clip(baseColor.a - 0.9);
    
    float3 N = normalize(input.normal);
    
    float2 roughnessMetallic = tRoughnessMetallic.Sample(linearWrapSampler, input.uv).gb;
    
    float3 L = normalize(volume.lightDir.xyz);
    
    float3 V = normalize(probeLocation - input.pos);
    
    float metallic = roughnessMetallic.g;
    
    float roughness = roughnessMetallic.r;
    
    float3 linearColor = pow(baseColor.rgb, 2.2);
    
    float3 albedo = lerp(linearColor, float3(0.0, 0.0, 0.0), metallic);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    
    F0 = lerp(F0, linearColor, metallic);
    
    float3 NdotL = saturate(dot(N, L));
    
    float dist = length(probeLocation - input.pos);
    
    float4 color = float4(PBR_BRDFEvaluate(N, V, L, F0, albedo, roughness) * volume.lightColor.rgb * NdotL * CalShadow(input.pos), 1.0);
    
    PixelOutput output;
    output.color = color;
    output.dist = dist;
    
    return output;
}
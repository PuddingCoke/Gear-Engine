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

cbuffer TextureIndex : register(DRAWCALLCONSTANTS)
{
    uint diffuseTexIndex;
    uint roughnessMetallicTexIndex;
    uint normalTexIndex;
    uint shadowTexIndex;
    uint irradianceVolumeBufIndex;
    uint irradianceOctahedralMapTexIndex;
    uint depthOctahedralMapTexIndex;
}

cbuffer ProjMatrices : register(DRAWCALLCBUFFER)
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

static Texture2DArray<float3> irradianceOctahedralMap = ResourceDescriptorHeap[irradianceOctahedralMapTexIndex];

static Texture2DArray<float2> depthOctahedralMap = ResourceDescriptorHeap[depthOctahedralMapTexIndex];

float CalShadow(float3 P)
{
    float4 shadowPos = mul(float4(P, 1.0), volume.lightViewProj);
    
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;
    
    return 1.0 - shadowTexture.SampleCmpLevelZero(shadowSampler, shadowPos.xy, shadowPos.z);
}

float4 main(PixelInput input) : SV_Target
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
    
    float3 color = PBR_BRDFEvaluate(N, V, L, F0, albedo, roughness) * volume.lightColor.rgb * NdotL * CalShadow(input.pos);
    
    const float PI = 3.14159265358979323846;
    
    color += albedo / PI * GetIndirectDiffuse(input.pos, N, volume, irradianceOctahedralMap, depthOctahedralMap, linearClampSampler);
    
    return float4(color, 1.0);
}
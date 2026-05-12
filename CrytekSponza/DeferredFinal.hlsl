#include"Common.hlsli"

#include"PBRHeader.hlsli"

#include"Utility.hlsli"

cbuffer TextureIndex : register(b2)
{
    uint gPositionMetallicTexIndex;
    uint gNormalRoughnessTexIndex;
    uint gBaseColorTexIndex;
    uint shadowTexIndex;
    uint irradianceOctahedralMapTexIndex;
    uint depthOctahedralMapTexIndex;
}

ConstantBuffer<IrradianceVolume> volume : register(b3);

static Texture2D gPositionMetallic = ResourceDescriptorHeap[gPositionMetallicTexIndex];

static Texture2D gNormalRoughness = ResourceDescriptorHeap[gNormalRoughnessTexIndex];

static Texture2D gBaseColor = ResourceDescriptorHeap[gBaseColorTexIndex];

static Texture2D<float> shadowTexture = ResourceDescriptorHeap[shadowTexIndex];

static Texture2DArray<float3> irradianceOctahedralMap = ResourceDescriptorHeap[irradianceOctahedralMapTexIndex];

static Texture2DArray<float2> depthOctahedralMap = ResourceDescriptorHeap[depthOctahedralMapTexIndex];

float CalShadow(float3 P)
{
    float4 shadowPos = mul(float4(P, 1.0), volume.lightViewProj);
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;
    
    float shadow = 0.0;
    const float2 texelSize = 1.0 / float2(4096.0, 4096.0);
    
    [unroll]
    for (int x = -1; x < 2; x++)
    {
        [unroll]
        for (int y = -1; y < 2; y++)
        {
            shadow += shadowTexture.SampleCmpLevelZero(shadowSampler, shadowPos.xy + float2(x, y) * texelSize, shadowPos.z);
        }
    }
    
    shadow /= 9.0;
    
    return 1.0 - shadow;
}

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float4 positionMetallic = gPositionMetallic.Sample(linearClampSampler, texCoord);
    
    if (positionMetallic.a > 1.2)
    {
        return float4(0.0, 0.0, 0.0, 1.0);
    }
    
    float4 normalRoughness = gNormalRoughness.Sample(linearClampSampler, texCoord);
    
    float3 baseColor = gBaseColor.Sample(linearClampSampler, texCoord).rgb;
    
    float3 position = positionMetallic.xyz;
    
    float3 outColor = float3(0.0, 0.0, 0.0);
    
    float3 N = normalize(normalRoughness.xyz);
    
    float3 L = normalize(volume.lightDir.xyz);
    
    float3 V = normalize(perframeResource.eyePos.xyz - position);
    
    float metallic = positionMetallic.a;
    
    float roughness = normalRoughness.a;
   
    float shadow = CalShadow(position);
    
    float3 linearColor = pow(baseColor, 2.2);
    
    float3 albedo = lerp(linearColor, float3(0.0, 0.0, 0.0), metallic);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    
    F0 = lerp(F0, linearColor, metallic);
    
    float3 NdotL = saturate(dot(N, L));
    
    outColor += PBR_BRDFEvaluate(N, V, L, F0, albedo, roughness) * volume.lightColor.rgb * NdotL * shadow;
    
    const float PI = 3.14159265358979323846;
    
    outColor += albedo / PI * GetIndirectDiffuse(position, N, volume, irradianceOctahedralMap, depthOctahedralMap, linearClampSampler);
    
    return float4(outColor, 1.0);
}
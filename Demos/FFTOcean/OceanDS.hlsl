#include"Common.hlsli"

struct DS_OUTPUT
{
    float2 texCoord : TEXCOORD;
    float4 lodScales : LODSCALES;
    float3 viewDir : VIEWDIR;
    float4 svPosition : SV_POSITION;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 position : POSITION;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint displacement0TextureIndex;
    uint displacement1TextureIndex;
    uint displacement2TextureIndex;
}

cbuffer RenderParam : register(PER_INVOKE_CBUFFER)
{
    float lodScale;
    float lengthScale0;
    float lengthScale1;
    float lengthScale2;
}

static Texture2D<float4> displacement0Texture = ResourceDescriptorHeap[displacement0TextureIndex];

static Texture2D<float4> displacement1Texture = ResourceDescriptorHeap[displacement1TextureIndex];

static Texture2D<float4> displacement2Texture = ResourceDescriptorHeap[displacement2TextureIndex];

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, 3> patch)
{
    DS_OUTPUT Output;

    float3 position = patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z;
    
    Output.texCoord = position.xz;
    
    Output.viewDir = perframeResource.eyePos.xyz - position;
    
    const float viewDist = length(Output.viewDir);
    
    const float lod_c0 = min(lodScale * lengthScale0 / viewDist, 1.0);
    
    const float lod_c1 = min(lodScale * lengthScale1 / viewDist, 1.0);
    
    const float lod_c2 = min(lodScale * lengthScale2 / viewDist, 1.0);
    
    position += displacement0Texture.SampleLevel(anisotrophicWrapSampler, position.xz / lengthScale0, 0.0).xyz * lod_c0;
    
    position += displacement1Texture.SampleLevel(anisotrophicWrapSampler, position.xz / lengthScale1, 0.0).xyz * lod_c1;
    
    position += displacement2Texture.SampleLevel(anisotrophicWrapSampler, position.xz / lengthScale2, 0.0).xyz * lod_c2;
    
    Output.lodScales = float4(lod_c0, lod_c1, lod_c2, 0.0);
    
    Output.svPosition = mul(float4(position, 1.0), perframeResource.viewProj);
    
    return Output;
}

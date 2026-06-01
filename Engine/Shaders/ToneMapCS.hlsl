#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint inputTextureIndex;
    uint outputTextureIndex;
    float exposure;
}

static Texture2D<float4> inputTexture = ResourceDescriptorHeap[inputTextureIndex];

static RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[outputTextureIndex];

float3 ACESToneMapping(float3 color)
{
    color *= exposure;
    
    const float A = 2.51;
    
    const float B = 0.03;
    
    const float C = 2.43;
    
    const float D = 0.59;
    
    const float E = 0.14;
    
    return (color * (A * color + B)) / (color * (C * color + D) + E);
}


[numthreads(16, 16, 1)]
void main( uint2 DTid : SV_DispatchThreadID )
{
    float3 color = max(inputTexture[DTid].rgb, float3(0.0, 0.0, 0.0));
    
    outputTexture[DTid] = float4(ACESToneMapping(color), 1.0);
}
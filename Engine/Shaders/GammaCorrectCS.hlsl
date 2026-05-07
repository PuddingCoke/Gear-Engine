cbuffer TextureIndices : register(b2)
{
    uint inputTextureIndex;
    uint outputTextureIndex;
    float gamma;
}

static Texture2D<float4> inputTexture = ResourceDescriptorHeap[inputTextureIndex];

static RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[outputTextureIndex];

[numthreads(16, 16, 1)]
void main( uint2 DTid : SV_DispatchThreadID )
{
    outputTexture[DTid] = float4(pow(inputTexture[DTid].rgb, 1.0 / gamma), 1.0);
}
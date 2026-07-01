//author:https://github.com/asylum2010

#include"Predefine.hlsli"

#include"Common.hlsli"

#define TWO_PI 6.283185307179586476925286766559

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint outputTextureIndex;
    uint inputTextureIndex;
}

static RWTexture2D<float2> outputTexture = ResourceDescriptorHeap[outputTextureIndex];

static Texture2D<float2> inputTexture = ResourceDescriptorHeap[inputTextureIndex];

groupshared float2 pingpong[2][TEXTURESIZE];

float2 ComplexMul(const float2 a, const float2 b)
{
    return float2(a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y);
}

[numthreads(TEXTURESIZE, 1, 1)]
void main(const uint groupThreadID : SV_GroupThreadID, const uint groupID : SV_GroupID)
{
    const float N = float(TEXTURESIZE);
    
    const int z = int(groupID);
    const int x = int(groupThreadID);
    
    const int nj = (reversebits(x) >> (32 - LOG2TEXTURESIZE)) & (TEXTURESIZE - 1);
    pingpong[0][nj] = inputTexture[int2(z, x)];
    
    GroupMemoryBarrierWithGroupSync();
    
    int src = 0;
    
    [unroll]
    for (int s = 1; s <= LOG2TEXTURESIZE; ++s)
    {
        const int m = 1 << s;
        const int mh = m >> 1;
        
        const int k = (x * (TEXTURESIZE / m)) & (TEXTURESIZE - 1);
        const int i = (x & ~(m - 1));
        const int j = (x & (mh - 1));
        
        const float theta = (TWO_PI * float(k)) / N;
        const float2 W_N_k = float2(cos(theta), sin(theta));
        
        const float2 input1 = pingpong[src][i + j + mh];
        const float2 input2 = pingpong[src][i + j];
        
        src = 1 - src;
        pingpong[src][x] = input2 + ComplexMul(W_N_k, input1);
        
        GroupMemoryBarrierWithGroupSync();
    }
    
    outputTexture[int2(x, z)] = pingpong[src][x];
}
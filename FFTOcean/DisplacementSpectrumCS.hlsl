#include"Common.hlsli"

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint DxDzIndex;
    uint DyDxzIndex;
    uint DyxDyzIndex;
    uint DxxDzzIndex;
    uint waveDataTextureIndex;
    uint waveSpectrumTextureIndex;
}

static RWTexture2D<float2> DxDz = ResourceDescriptorHeap[DxDzIndex];

static RWTexture2D<float2> DyDxz = ResourceDescriptorHeap[DyDxzIndex];

static RWTexture2D<float2> DyxDyz = ResourceDescriptorHeap[DyxDyzIndex];

static RWTexture2D<float2> DxxDzz = ResourceDescriptorHeap[DxxDzzIndex];

static Texture2D<float4> waveDataTexture = ResourceDescriptorHeap[waveDataTextureIndex];

static Texture2D<float4> waveSpectrumTexture = ResourceDescriptorHeap[waveSpectrumTextureIndex];

float2 ComplexMul(const float2 a, const float2 b)
{
    return float2(a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y);
}

[numthreads(8, 8, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float4 waveData = waveDataTexture[DTid];
    
    const float4 waveSpectrum = waveSpectrumTexture[DTid];
    
    const float2 tildeh0k = waveSpectrum.xy;
    
    const float2 tildeh0MinuskConj = waveSpectrum.zw;
    
    const float omegat = waveData.w * perframeResource.timeElapsed;
    
    const float cos_ = cos(omegat);
    
    const float sin_ = sin(omegat);
    
    const float2 c0 = float2(cos_, sin_);
    
    const float2 c1 = float2(cos_, -sin_);
    
    const float2 h = ComplexMul(tildeh0k, c0) + ComplexMul(tildeh0MinuskConj, c1);
    
    const float2 ih = float2(-h.y, h.x);
    
    const float2 displacementY = h;
    
    const float2 displacementY_dx = ih * waveData.x;
    
    const float2 displacementY_dz = ih * waveData.z;
    
    const float2 displacementX = ih * waveData.x * waveData.y;
    
    const float2 displacementZ = ih * waveData.z * waveData.y;
    
    const float2 displacementX_dx = -h * waveData.x * waveData.x * waveData.y;
    
    const float2 displacementZ_dz = -h * waveData.z * waveData.z * waveData.y;
    
    const float2 displacementX_dz = -h * waveData.x * waveData.z * waveData.y;
    
    DxDz[DTid] = float2(displacementX.x - displacementZ.y, displacementX.y + displacementZ.x);
    
    DyDxz[DTid] = float2(displacementY.x - displacementX_dz.y, displacementY.y + displacementX_dz.x);
    
    DyxDyz[DTid] = float2(displacementY_dx.x - displacementY_dz.y, displacementY_dx.y + displacementY_dz.x);
    
    DxxDzz[DTid] = float2(displacementX_dx.x - displacementZ_dz.y, displacementX_dx.y + displacementZ_dz.x);
}
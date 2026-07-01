#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint displacementTextureIndex;
    uint derivativeTextureIndex;
    uint jacobianTextureIndex;
    uint DxDzIndex;
    uint DyDxzIndex;
    uint DyxDyzIndex;
    uint DxxDzzIndex;
}

static RWTexture2D<float4> displacementTexture = ResourceDescriptorHeap[displacementTextureIndex];

static RWTexture2D<float4> derivativeTexture = ResourceDescriptorHeap[derivativeTextureIndex];

static RWTexture2D<float> jacobianTexture = ResourceDescriptorHeap[jacobianTextureIndex];

static Texture2D<float2> DxDz = ResourceDescriptorHeap[DxDzIndex];

static Texture2D<float2> DyDxz = ResourceDescriptorHeap[DyDxzIndex];

static Texture2D<float2> DyxDyz = ResourceDescriptorHeap[DyxDyzIndex];

static Texture2D<float2> DxxDzz = ResourceDescriptorHeap[DxxDzzIndex];

static const float lambda = 1.0;

[numthreads(8, 8, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float2 Dx_Dz = DxDz[DTid];
    
    const float2 Dy_Dxz = DyDxz[DTid];
    
    const float2 Dyx_Dyz = DyxDyz[DTid];
    
    const float2 Dxx_Dzz = DxxDzz[DTid];
    
    displacementTexture[DTid] = float4(lambda * Dx_Dz.x, Dy_Dxz.x, lambda * Dx_Dz.y, 1.0);
    
    derivativeTexture[DTid] = float4(Dyx_Dyz, lambda * Dxx_Dzz);
    
    float jacobian = (1.0 + lambda * Dxx_Dzz.x) * (1.0 + lambda * Dxx_Dzz.y) - lambda * lambda * Dy_Dxz.y * Dy_Dxz.y;
    
    const float temp = jacobianTexture[DTid] + perframeResource.deltaTime * 0.5 / max(jacobian, 0.5);
    
    jacobian = min(jacobian, temp);
    
    jacobianTexture[DTid] = jacobian;
}
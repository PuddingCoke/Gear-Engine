#include"Common.hlsli"

cbuffer TextureIdx : register(DRAWCALLCONSTANTS)
{
    uint rgbaTexIdx;
    uint lumaTexIdx;
    uint chromaTexIdx;
};

static Texture2D<float4> RGBATex = ResourceDescriptorHeap[rgbaTexIdx];

static RWTexture2D<float> LumaTex = ResourceDescriptorHeap[lumaTexIdx];

static RWTexture2D<float2> ChromaTex = ResourceDescriptorHeap[chromaTexIdx];

float3 RGBToYUV(const float3 rgb)
{
    float Y = 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b;

    float U = 0.564 * (rgb.b - Y) + 0.5;

    float V = 0.713 * (rgb.r - Y) + 0.5;

    return float3(Y, U, V);
}

[numthreads(16, 16, 1)]
void main(uint2 chromaWriteCoord : SV_DispatchThreadID)
{
    const uint2 lumaWriteCoord = chromaWriteCoord * uint2(2, 2);
    
    const float3 yuv00 = RGBToYUV(RGBATex[lumaWriteCoord + uint2(0, 0)].rgb);
    
    const float3 yuv10 = RGBToYUV(RGBATex[lumaWriteCoord + uint2(1, 0)].rgb);
    
    const float3 yuv01 = RGBToYUV(RGBATex[lumaWriteCoord + uint2(0, 1)].rgb);
    
    const float3 yuv11 = RGBToYUV(RGBATex[lumaWriteCoord + uint2(1, 1)].rgb);

    LumaTex[lumaWriteCoord + uint2(0, 0)] = yuv00.r;
    
    LumaTex[lumaWriteCoord + uint2(1, 0)] = yuv10.r;
    
    LumaTex[lumaWriteCoord + uint2(0, 1)] = yuv01.r;
    
    LumaTex[lumaWriteCoord + uint2(1, 1)] = yuv11.r;
    
    const float2 uv = (yuv00.gb + yuv10.gb + yuv01.gb + yuv11.gb) * 0.25;
    
    ChromaTex[chromaWriteCoord] = uv;
}
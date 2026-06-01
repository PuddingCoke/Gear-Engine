#define M_PI 3.1415926535897932384626433832795

#include"Common.hlsli"

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint tildeh0TextureIndex;
    uint waveDataTextureIndex;
    uint randomGaussTextureIndex;
}

cbuffer SpectrumParam : register(DRAWCALLCBUFFER)
{
    uint mapResolution;
    float mapLength;
    float2 wind;
    float amplitude;
    float gravity;
    float cutoffLow;
    float cutoffHigh;
}

static RWTexture2D<float2> tildeh0Texture = ResourceDescriptorHeap[tildeh0TextureIndex];

static RWTexture2D<float4> waveDataTexture = ResourceDescriptorHeap[waveDataTextureIndex];

static Texture2D<float4> randomGaussTexture = ResourceDescriptorHeap[randomGaussTextureIndex];

float phillips(const float2 k)
{
    const float L = length(wind) * length(wind) / gravity;
    
    const float kSquare = dot(k, k);
    
    const float kdotw = dot(normalize(k), normalize(wind));
    
    float P_h = amplitude * exp(-1.0 / (kSquare * L * L)) / (kSquare * kSquare) * (kdotw * kdotw);
    
    //wave move against wind direction
    if (kdotw < 0.0)
    {
        P_h *= 0.07;
    }
    
    const float l = L * 0.001;
    
    //supress very small wave multiply exp(-k^2 * l^2)
    return P_h * exp(-kSquare * l * l);
}

//deep water
float waveFrequency(const float2 k)
{
    return sqrt(gravity * length(k));
}

[numthreads(8, 8, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float2 k = float2(M_PI * (2.0 * float(DTid.x) - float(mapResolution)) / float(mapLength),
                            M_PI * (2.0 * float(DTid.y) - float(mapResolution)) / float(mapLength));
    
    const float len = length(k);
    
    if (len >= cutoffLow && len <= cutoffHigh)
    {
        tildeh0Texture[DTid] = randomGaussTexture[DTid].xy * sqrt(phillips(k) / 2.0);

        waveDataTexture[DTid] = float4(k.x, 1.0 / len, k.y, waveFrequency(k));
    }
    else
    {
        tildeh0Texture[DTid] = float2(0.0, 0.0);

        waveDataTexture[DTid] = float4(k.x, 1.0, k.y, 0.0);
    }
}
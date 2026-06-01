#include"Common.hlsli"

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint preStateTextureIndex;
    uint curStateTextureIndex;
    float ra;
    float ri;
    float b1;
    float d1;
    float b2;
    float d2;
    float alpha_n;
    float alpha_m;
};

static Texture2D<float> preStateTexture = ResourceDescriptorHeap[preStateTextureIndex];

static RWTexture2D<float> curStateTexture = ResourceDescriptorHeap[curStateTextureIndex];

float sigma_1(float x, float a, float alpha)
{
    return 1.0 / (1.0 + exp(-(x - a) * 4.0 / alpha));
}

float sigma_2(float x, float a, float b, float alpha)
{
    return sigma_1(x, a, alpha) * (1.0 - sigma_1(x, b, alpha));
}

float sigma_m(float x, float y, float m, float alpha)
{
    return x * (1.0 - sigma_1(m, 0.5, alpha)) + y * sigma_1(m, 0.5, alpha);
}

float s(float n, float m)
{
    return sigma_2(n, sigma_m(b1, d1, m, alpha_m), sigma_m(b2, d2, m, alpha_m), alpha_n);
}

float AAFactor(float r, float l)
{
    return saturate(r - l + 0.5);
}

#define PI 3.1415926535897932384626433832795

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    const float M = PI * ri * ri;
    
    const float N = PI * (ra * ra - ri * ri);
    
    float m = 0.0;
    
    float n = 0.0;
    
    float2 resolution;
    
    preStateTexture.GetDimensions(resolution.x, resolution.y);
    
    for (float dx = -ra; dx <= ra; dx++)
    {
        for (float dy = -ra; dy <= ra; dy++)
        {
            const float radius = length(float2(dx, dy));
            
            const float2 uv = (float2(DTid) + float2(dx + 0.5, dy + 0.5)) / resolution;

            const float value = preStateTexture.SampleLevel(linearWrapSampler, uv, 0.0);
            
            m += value * AAFactor(ri, radius);
            
            n += value * AAFactor(ra, radius) * AAFactor(radius, ri);
        }
    }
    
    m /= M;
    
    n /= N;
    
    curStateTexture[DTid] = s(n, m);
}
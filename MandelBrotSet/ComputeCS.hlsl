#include"Common.hlsli"

cbuffer TextureIndices : register(DRAWCALLCONSTANTS)
{
    uint outputTextureIndex;
    float2 location;
    float scale;
    float2 texelSize;
    float lerpFactor;
    float lerpFactor2;
    uint frameIndex;
    float floatSeed;
}

static RWTexture2D<float4> outputTexture = ResourceDescriptorHeap[outputTextureIndex];

#define MAXITERATION 500

#define ESCAPEBOUNDARY 16.0

float2 ComplexSqr(in const float2 c)
{
    return float2(c.x * c.x - c.y * c.y, 2.0 * c.x * c.y);
}

static const float values[] = { 0.0, 0.16, 0.42, 0.6425, 0.8575, 1.0 };

static const float3 colors[] =
{
    { 0, 7, 100 },
    { 32, 107, 203 },
    { 237, 255, 255 },
    { 255, 170, 0 },
    { 0, 2, 0 },
    { 0, 7, 100 },
};

static const float3 tangent[] =
{
    { 200, 625, 643.75 },
    { 494.231, 597.115, 421.875 },
    { 434.68, 93.6041, -473.034 },
    { -552.574, -581.709, 0 },
    { 0, -373.154, 0 },
    { 0, 35.0877, 701.754 }
};

float3 interpolateColor(float t)
{
    [unroll]
    for (uint i = 0; i < 6; i++)
    {
        if (values[i + 1] >= t)
        {
            t = (t - values[i]) / (values[i + 1] - values[i]);
            
            return (2 * pow(t, 3) - 3 * pow(t, 2) + 1) * colors[i] +
					(pow(t, 3) - 2 * pow(t, 2) + t) * (values[i + 1] - values[i]) * tangent[i] +
					(-2 * pow(t, 3) + 3 * pow(t, 2)) * colors[i + 1] +
					(pow(t, 3) - pow(t, 2)) * (values[i + 1] - values[i]) * tangent[i + 1];
        }
    }

    return float3(0.0, 0.0, 0.0);
}

float2 ComplexMul(const float2 a, const float2 b)
{
    return float2(a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y);
}

static float hashSeed = 0.0;

uint BaseHash(uint2 p)
{
    p = 1103515245U * ((p >> 1U) ^ (p.yx));
    uint h32 = 1103515245U * ((p.x) ^ (p.y >> 3U));
    return h32 ^ (h32 >> 16);
}

float2 Hash2(inout float seed)
{
    uint n = BaseHash(asuint(float2(seed += 0.1, seed += 0.1)));
    uint2 rz = uint2(n, n * 48271U);
    return float2(rz.xy & uint2(0x7fffffffU, 0x7fffffffU)) / float(0x7fffffff);
}

[numthreads(16, 9, 1)]
void main(const uint2 DTid : SV_DispatchThreadID)
{
    hashSeed = float(BaseHash(asuint(float2(DTid) + float2(0.5, 0.5)))) / float(0xffffffffU) + floatSeed;
    
    const float2 originPosition = ((float2(DTid) + float2(0.5, 0.5) + Hash2(hashSeed)) * texelSize - 0.5) * float2(2.2 * 16.0 / 9.0, 2.2) * scale + location;
    
    float2 z = originPosition;
    
    float2 dz = float2(1.0, 0.0);
    
    const float2 c = float2(-0.5251993, -0.5251993);
    
    //0.232366 0.562292
    //0.000009
    
    uint i = 0;
    
    uint reason = 0;
    
    const float pixelSize = 1e-8;
    
    float power = 1.0;
    
    [unroll]
    for (; i < MAXITERATION; i++)
    {
        z = ComplexSqr(z) + c;
        
        dz = 2.0 * ComplexMul(z, dz);
        
        power = power * 2.0;
        
        if (dot(z, z) < dot(dz * pixelSize, dz * pixelSize))
        {
            reason = 1;
            
            break;
        }
        
        if (dot(z, z) > ESCAPEBOUNDARY)
        {
            reason = 2;
            
            break;
        }
    }
    
    const float smoothed_i = float(i) - log2(max(1.0, log2(length(z))));
    
    const float iter_ratio = saturate(smoothed_i / float(MAXITERATION));
    
    float3 color = float3(0.0, 0.0, 0.0);
    
    if (reason == 1)
    {
        color = interpolateColor(iter_ratio * lerpFactor) / 255.0;
    }
    else if (reason == 2)
    {
        const float dist = 2.0 * log(length(z)) * length(z) / length(dz);
        
        const float t = saturate(tanh(dist * 1080.0));
        
        color = interpolateColor(iter_ratio * lerpFactor2) / 255.0 * t;
    }
    
    float fadeFactor = 1.0 - log(dot(z, z)) / power;
    
    if(fadeFactor<0.f)
    {
        fadeFactor = 0.f;
    }
    
    color *= fadeFactor;
    
    outputTexture[DTid] = lerp(outputTexture[DTid], float4(color, 1.0), 1.0 / float(frameIndex));
}
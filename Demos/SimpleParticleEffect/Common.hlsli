#define ENGINE_GLOBAL_CBUFFER b0
#define USER_GLOBAL_CBUFFER b1
#define PER_INVOKE_CONSTANTS b2
#define PER_INVOKE_CBUFFER b3

struct PerframeResource
{
    float deltaTime;
    float timeElapsed;
    uint uintSeed;
    float floatSeed;
    matrix proj;
    matrix view;
    float4 eyePos;
    matrix prevViewProj;
    matrix viewProj;
    matrix normalMatrix;
    float2 screenSize;
    float2 screenTexelSize;
};

ConstantBuffer<PerframeResource> perframeResource : register(ENGINE_GLOBAL_CBUFFER);

SamplerState pointClampSampler : register(s0);
SamplerState pointWrapSampler : register(s1);
SamplerState linearClampSampler : register(s2);
SamplerState linearWrapSampler : register(s3);
SamplerState anisotrophicClampSampler : register(s4);
SamplerState anisotrophicWrapSampler : register(s5);
SamplerComparisonState shadowSampler : register(s6);

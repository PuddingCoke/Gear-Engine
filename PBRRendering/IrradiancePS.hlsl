#include"Common.hlsli"

cbuffer TextureIndex : register(PER_INVOKE_CONSTANTS)
{
    uint textureIndex;
}

static TextureCube envMap = ResourceDescriptorHeap[textureIndex];

#define PI 3.14159265358979323846

float4 main(float3 position : POSITION) : SV_TARGET
{
    uint thetaSamples = 128;
	
    uint phiSamples = 128;
	
    float deltaTheta = PI / 2.0 / thetaSamples;
    
    float deltaPhi = 2.0 * PI / phiSamples;
    
    float3 N = normalize(position);
    
    float3 upVector = abs(dot(float3(0.0, 0.0, 1.0), N)) > abs(dot(float3(1.0, 0.0, 0.0), N)) ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    
    float3 T = normalize(cross(N, upVector));
    
    float3 B = normalize(cross(T, N));
    
    float3x3 TBN = float3x3(T, B, N);
    
    float3 irradiance = float3(0.0, 0.0, 0.0);
    
    for (uint i = 0; i < thetaSamples; i++)
    {
        for (uint j = 0; j < phiSamples; j++)
        {
            float theta = deltaTheta * float(i);
            
            float phi = deltaPhi * float(j);
            
            float cosTheta = cos(theta);
            
            float sinTheta = sin(theta);
            
            float x = sinTheta * sin(phi);
            
            float y = sinTheta * cos(phi);
            
            float z = cosTheta;
            
            float3 sampleVec = normalize(mul(float3(x, y, z), TBN));
            
            float deltaSR = deltaTheta * deltaPhi * sinTheta;
            
            irradiance += envMap.Sample(linearWrapSampler, sampleVec).rgb * cosTheta * deltaSR;
        }
    }
    
    return float4(irradiance, 1.0f);
}
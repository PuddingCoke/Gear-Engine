#include"Common.hlsli"

#include"Utility.hlsli"

cbuffer TextureIndex : register(b2)
{
    uint irradianceOctahedralMapTexIndex;
    uint envCubeTexIndex;
}

cbuffer ProjMatrices : register(b3)
{
    matrix viewProj[6];
    float3 probeLocation;
    uint probeIndex;
}

static RWTexture2DArray<float3> irradianceOctahedralMap = ResourceDescriptorHeap[irradianceOctahedralMapTexIndex];

static TextureCube envCube = ResourceDescriptorHeap[envCubeTexIndex];

#define PI 3.14159265358979323846

[numthreads(6, 6, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
    bool isBorderTexel = (DTid.x == 0 || DTid.x == 5) | (DTid.y == 0 || DTid.y == 5);
    
    if (!isBorderTexel)
    {
        const float2 coor = (float2(DTid - uint2(1, 1)) + 0.5) / 2.0 - float2(1.0, 1.0);
    
        uint thetaSamples = 32;
	
        uint phiSamples = 32;
	
        float deltaTheta = PI / 2.0 / thetaSamples;
    
        float deltaPhi = 2.0 * PI / phiSamples;
    
        float3 N = octDecode(coor.x, coor.y);
    
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
            
                irradiance += envCube.Sample(linearWrapSampler, sampleVec).rgb * cosTheta * deltaSR;
            }
        }
        
        irradiance = 1.0989 * irradiance;
        
        irradianceOctahedralMap[uint3(DTid, probeIndex)] = irradiance;
    }
    
    AllMemoryBarrierWithGroupSync();
    
    if (isBorderTexel)
    {
        bool isCornerTexel = (DTid.x == 0 || DTid.x == 5) && (DTid.y == 0 || DTid.y == 5);
        bool isRowTexel = (DTid.x > 0 && DTid.x < 5);

        uint2 copyCoor;

        if (isCornerTexel)
        {
            copyCoor.x = DTid.x > 0 ? 1 : 4;
            copyCoor.y = DTid.y > 0 ? 1 : 4;
        }
        else if (isRowTexel)
        {
            copyCoor.x = 5 - DTid.x;
            copyCoor.y = DTid.y + ((DTid.y > 0) ? -1 : 1);
        }
        else
        {
            copyCoor.x = DTid.x + ((DTid.x > 0) ? -1 : 1);
            copyCoor.y = 5 - DTid.y;
        }

        irradianceOctahedralMap[uint3(DTid, probeIndex)] = irradianceOctahedralMap[uint3(copyCoor, probeIndex)];
    }
}
struct IrradianceVolume
{
    float4 lightDir;
    float4 lightColor;
    matrix lightViewProj;
    float3 fieldStart;
    float probeSpacing;
    uint3 probeCount;
    float irradianceDistanceBias;
    float irradianceVarianceBias;
    float irradianceChebyshevBias;
};

float signNotZero(float k)
{
    return k >= 0.0 ? 1.0 : -1.0;
}

float2 signNotZero(float2 v)
{
    return float2(signNotZero(v.x), signNotZero(v.y));
}

float3 octDecode(float x, float y)
{
    float3 v = float3(x, y, 1.0 - abs(x) - abs(y));
    
    if (v.z < 0)
    {
        v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
    }
    
    return normalize(v);
}

float2 octEncode(float3 v)
{
    float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    
    float2 result = v.xy * (1.0 / l1norm);
    
    if (v.z < 0.0)
    {
        result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
    }
    
    return result;
}

float2 GetDepth(in float3 N, in uint probeIndex, in Texture2DArray<float2> depthOctahedralMap, in SamplerState samplerState)
{
    float2 texCoord = octEncode(N);
    
    //convert [-1,1] to [1/16,15/16]
    
    texCoord = (texCoord + 1.0) / 2.0;
    
    texCoord = (texCoord * 14.0 + 1.0) / 16.0;
    
    return depthOctahedralMap.Sample(samplerState, float3(texCoord, float(probeIndex)));
}

float3 GetIrradiance(in float3 N, in uint probeIndex, in Texture2DArray<float3> irradianceOctahedralMap,in SamplerState samplerState)
{
    float2 texCoord = octEncode(N);
    
    //convert [-1,1] to [1/6,5/6]
    
    texCoord = (texCoord + 1.0) / 2.0;
    
    texCoord = (texCoord * 4.0 + 1.0) / 6.0;
    
    return irradianceOctahedralMap.Sample(samplerState, float3(texCoord, float(probeIndex)));
}

uint3 PosToProbeGridPos(in float3 P, in ConstantBuffer<IrradianceVolume> volume)
{
    return uint3(clamp(int3((P - volume.fieldStart) / volume.probeSpacing), int3(0, 0, 0), int3(volume.probeCount - 1)));
}

uint ProbeGridPosToIndex(in uint3 probeGridPos, in ConstantBuffer<IrradianceVolume> volume)
{
    return probeGridPos.x + probeGridPos.z * volume.probeCount.x + probeGridPos.y * volume.probeCount.x * volume.probeCount.z;
}

float3 ProbeGridPosToLoc(in uint3 probeGridPos, in ConstantBuffer<IrradianceVolume> volume)
{
    return volume.fieldStart + float3(probeGridPos) * volume.probeSpacing;
}

float3 GetIndirectDiffuse(in float3 P, in float3 N, in ConstantBuffer<IrradianceVolume> volume, in Texture2DArray<float3> irradianceOctahedralMap, in Texture2DArray<float2> depthOctahedralMap, in SamplerState samplerState)
{
    float3 sumIrradiance = float3(0.0, 0.0, 0.0);
    
    float sumWeight = 0.0;
    
    uint3 baseGridCoord = PosToProbeGridPos(P, volume);
    
    float3 baseProbePos = ProbeGridPosToLoc(baseGridCoord, volume);
    
    float3 alpha = clamp((P - baseProbePos) / volume.probeSpacing, float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0));
    
    [unroll]
    for (uint i = 0; i < 8; i++)
    {
        uint3 offset = uint3(i, i >> 1, i >> 2) & uint3(1, 1, 1);
        
        uint3 probeGridCoord = clamp(baseGridCoord + offset, uint3(0, 0, 0), uint3(volume.probeCount - 1));
        
        uint probeIndex = ProbeGridPosToIndex(probeGridCoord, volume);
        
        float3 probePos = ProbeGridPosToLoc(probeGridCoord, volume);
        
        float3 probeToPoint = P - probePos;
        
        float3 dir = normalize(-probeToPoint);
        
        float distToProbe = length(probeToPoint);
        
        float3 trilinear = lerp(float3(1.0, 1.0, 1.0) - alpha, alpha, float3(offset));
        
        float weight = (trilinear.x * trilinear.y * trilinear.z) * max(0.005, dot(dir, N));
        
        float2 temp = GetDepth(-dir, probeIndex, depthOctahedralMap, samplerState);
        
        float mean = temp.x + volume.irradianceDistanceBias;
        
        float variance = abs(temp.x * temp.x - temp.y) + volume.irradianceVarianceBias;
        
        float chebyshevWeight = variance / (variance + (distToProbe - mean) * (distToProbe - mean));
        
        chebyshevWeight = max(chebyshevWeight * chebyshevWeight - volume.irradianceChebyshevBias, 0.0) / (1.0 - volume.irradianceChebyshevBias);
        
        weight = max(0.00001, weight * ((distToProbe <= mean) ? 1.0 : chebyshevWeight));
        
        sumWeight += weight;
        
        float3 irradiance = GetIrradiance(N, probeIndex, irradianceOctahedralMap, samplerState);
        
        //调试用途
        if (isnan(irradiance.x) || isnan(irradiance.y) || isnan(irradiance.z))
        {
            return float3(1.0, 0.0, 0.0);
        }
        
        sumIrradiance += weight * GetIrradiance(N, probeIndex, irradianceOctahedralMap, samplerState);
    }
    
    return sumIrradiance / sumWeight;
}
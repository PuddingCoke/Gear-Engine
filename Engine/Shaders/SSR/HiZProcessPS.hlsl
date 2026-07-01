#include"Common.hlsli"

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint gPositionTexIndex;
    uint gNormalTexIndex;
    uint hiZTexIndex;
    int maxLevel;
}

static Texture2D<float4> gPosition = ResourceDescriptorHeap[gPositionTexIndex];

static Texture2D<float4> gNormal = ResourceDescriptorHeap[gNormalTexIndex];

static Texture2D<float> hiZTexture = ResourceDescriptorHeap[hiZTexIndex];

float3 IntersectDepthPlane(in float3 o, in float3 d, in float t)
{
    return o + d * t;
}

float3 IntersectCellBoundary(in float3 o, in float3 d, in float2 cell, in float2 cell_count, in float2 crossStep, in float2 crossOffset)
{
    float3 intersection = 0;
	
    float2 index = cell + crossStep;
    float2 boundary = index / cell_count;
    boundary += crossOffset;
	
    float2 delta = boundary - o.xy;
    delta /= d.xy;
    float t = min(delta.x, delta.y);
	
    intersection = IntersectDepthPlane(o, d, t);
	
    return intersection;
}

float2 GetCell(in float2 pos, in float2 cellCount)
{
    return floor(pos * cellCount);
}

float2 GetCellCount(in int mipLevel)
{
    float3 dimension;
    
    hiZTexture.GetDimensions(mipLevel, dimension.x, dimension.y, dimension.z);
    
    return float2(dimension.xy);
}

float GetMinimumDepthPlane(in float2 p, in int mipLevel)
{
    return hiZTexture.SampleLevel(pointClampSampler, p, float(mipLevel));
}

bool CrossedCellBoundary(in float2 oldCellIndex, in float2 newCellIndex)
{
    return (oldCellIndex.x != newCellIndex.x) || (oldCellIndex.y != newCellIndex.y);
}

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    float4 pos = gPosition.Sample(pointClampSampler, texCoord);
    
    if (pos.w > 1.2)
    {
        return float4(0.0, 0.0, 0.0, 1.0);
    }
    
    pos.w = 1.0;
    
    const float4 positionFrom = mul(pos, perframeResource.view);
    
    float3 normal = gNormal.Sample(pointClampSampler, texCoord).xyz;
    
    const float3 unitPositionFrom = normalize(positionFrom.xyz);
    
    normal = normalize(mul(normal, (float3x3) perframeResource.normalMatrix));
    
    const float3 pivot = normalize(reflect(unitPositionFrom, normal));
    
    float4 startView = float4(positionFrom.xyz, 1.0);
    
    float4 endView = float4(positionFrom.xyz + (pivot * 150.0), 1.0);
    
    float4 startFrag = mul(startView, perframeResource.proj);
    startFrag /= startFrag.w;
    startFrag.xy = startFrag.xy * float2(0.5, -0.5) + 0.5;
    
    float4 endFrag = mul(endView, perframeResource.proj);
    endFrag /= endFrag.w;
    endFrag.xy = endFrag.xy * float2(0.5, -0.5) + 0.5;
    
    const float3 percentage = (saturate(endFrag.xyz) - startFrag.xyz) / (endFrag.xyz - startFrag.xyz);
    
    const float minPercentage = saturate(min(min(percentage.x, percentage.y), percentage.z));
    
    endFrag = lerp(startFrag, endFrag, minPercentage);
    
    const float3 reflectDir = normalize(endFrag.xyz - startFrag.xyz);
    
    float2 crossStep = float2(reflectDir.x >= 0 ? 1 : -1, reflectDir.y >= 0 ? 1 : -1);
    
    const float2 crossOffset = crossStep / GetCellCount(0) / 128.0;
    
    //crossStep = saturate(crossStep);
    
    float3 ray = startFrag.xyz;
    
    const float minZ = startFrag.z;
    
    const float maxZ = endFrag.z;
    
    const float deltaZ = (maxZ - minZ);
    
    const float3 o = startFrag.xyz;
    
    const float3 d = endFrag.xyz - startFrag.xyz;
    
    const int startLevel = maxLevel;
    
    const int stopLevel = 0;
    
    const float2 startCellCount = GetCellCount(startLevel);
    
    const float2 rayCell = GetCell(ray.xy, startCellCount);
    
    ray = IntersectCellBoundary(o, d, rayCell, startCellCount, crossStep, crossOffset * 64.0);
    
    int level = startLevel;
    
    uint iteration = 0;
    
    const bool isBackwardRay = (reflectDir.z < 0);
    
    const float rayDir = isBackwardRay ? -1 : 1;
    
    [loop]
    while (level >= stopLevel && ray.z * rayDir <= maxZ * rayDir && iteration < 100)
    {
        const float2 cellCount = GetCellCount(level);
        
        const float2 oldCellIdx = GetCell(ray.xy, cellCount);
        
        float cell_minZ = GetMinimumDepthPlane((oldCellIdx + 0.5) / cellCount, level);
        
        float3 tmpRay = ((cell_minZ > ray.z) && !isBackwardRay) ? IntersectDepthPlane(o, d, (cell_minZ - minZ) / deltaZ) : ray;
        
        const float2 newCellIdx = GetCell(tmpRay.xy, cellCount);
        
        float thickness = ((level == 0) ? (ray.z - cell_minZ) : 0);
        
        bool crossed = (isBackwardRay && (cell_minZ > ray.z)) || (thickness > 0.001) || CrossedCellBoundary(oldCellIdx, newCellIdx);
        
        ray = crossed ? IntersectCellBoundary(o, d, oldCellIdx, cellCount, crossStep, crossOffset) : tmpRay;
        
        level = crossed ? min(maxLevel, level + 1) : (level - 1);
        
        iteration++;
    }
    
    if (level < stopLevel)
    {
        const float4 positionTo = mul(float4(gPosition.Sample(pointClampSampler, ray.xy).xyz, 1.0), perframeResource.view);
    
        const float3 hitNormal = normalize(mul(gNormal.Sample(pointClampSampler, ray.xy).xyz, (float3x3) perframeResource.normalMatrix));
    
        const float visibility =
        saturate((1.0 - max(dot(-unitPositionFrom, pivot), 0))
        * (1.0 - saturate(length(positionTo - positionFrom) / 150.0))
        * (dot(hitNormal, pivot) < 0.0 ? 1.0 : 0.0));
        
        return float4(ray.xy, visibility, 1.0);
    }
    
    return float4(0.0, 0.0, 0.0, 1.0);
}
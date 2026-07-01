#include"Common.hlsli"

#define MAXITERATION 100
#define LEVELOFDETAIL 0.0005
#define FOVANGLEY 0.78539816339744

#define RAYMARCHITERATION 150

struct DrawCallParam
{
    uint frameIndex;
    float floatSeed;
    float phi;
    float theta;
    float radius;
    float POWER;
};

ConstantBuffer<DrawCallParam> drawCallParam : register(PER_INVOKE_CONSTANTS);

static float hashSeed = 0.0;

#define TWO_PI 6.283185307179586476925286766559

uint BaseHash(uint2 p)
{
    p = 1103515245U * ((p >> 1U) ^ (p.yx));
    uint h32 = 1103515245U * ((p.x) ^ (p.y >> 3U));
    return h32 ^ (h32 >> 16);
}

float Hash1(inout float seed)
{
    uint n = BaseHash(asuint(float2(seed += 0.1, seed += 0.1)));
    return float(n) * (1.0 / float(0xffffffffU));
}

float2 Hash2(inout float seed)
{
    uint n = BaseHash(asuint(float2(seed += 0.1, seed += 0.1)));
    uint2 rz = uint2(n, n * 48271U);
    return float2(rz.xy & uint2(0x7fffffffU, 0x7fffffffU)) / float(0x7fffffff);
}

float3 Hash3(inout float seed)
{
    uint n = BaseHash(asuint(float2(seed += 0.1, seed += 0.1)));
    uint3 rz = uint3(n, n * 16807U, n * 48271U);
    return float3(rz & uint3(0x7fffffffU, 0x7fffffffU, 0x7fffffffU)) / float(0x7fffffff);
}

float3 RandomPointInUnitSphere(inout float seed)
{
    float3 h = Hash3(seed) * float3(TWO_PI, 2.0, 1.0) - float3(0.0, 1.0, 0.0);
    float theta = h.x;
    float sinPhi = sqrt(1.0 - h.y * h.y);
    float r = pow(h.z, 0.3333333334);
    
    return r * float3(cos(theta) * sinPhi, sin(theta) * sinPhi, h.y);
}

float SDF(const float3 pos)
{
    float3 z = pos;
    
    float r;
    float dr = 1.0;
    
    [loop]
    for (uint i = 0; i < 20; i++)
    {
        r = length(z);
        
        if (r > 4.0)
            break;
        
        const float theta = asin(z.z / r) * drawCallParam.POWER;
        const float phi = atan2(z.y, z.x) * drawCallParam.POWER;
        
        dr = pow(r, drawCallParam.POWER - 1.0) * drawCallParam.POWER * dr + 1.0;
        const float zr = pow(r, drawCallParam.POWER);
        
        z = float3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta)) * zr + pos;
    }
    
    return log(r) * r / dr * 0.5;
}

float3 GetNormal(float3 pos)
{
    float2 eps = float2(0.0, 0.00001);
    
    float3 normal = normalize(float3(
    SDF(pos + eps.yxx) - SDF(pos - eps.yxx),
   SDF(pos + eps.xyx) - SDF(pos - eps.xyx),
    SDF(pos + eps.xxy) - SDF(pos - eps.xxy)));

    return normal;
}

float ShadowRayMarch(float3 P, float3 D)
{
    [loop]
    for (uint i = 0; i < 50; i++)
    {
        float dist = SDF(P);
        
        P += dist * D;
        
        if (dist < 0.005)
        {
            return 0.0;
        }
    }
    
    return 1.0;
}

static const float3 materialColor = float3(0.65, 0.65, 0.65);

static const float3 L = normalize(float3(1.0, 1.0, 1.0));

float3 GetBackgroundColor(in float z)
{
    return lerp(float3(1.0, 1.0, 1.0), float3(0.5, 0.7, 1.0), z);
}

float3 ShadeRay(float3 P, float3 N, float3 D)
{
    float3 diffuseSun = float3(1.0, 1.0, 1.0) * max(dot(N, L), 0.0) * ShadowRayMarch(P + 0.05 * L, L);
   
    float3 diffuseSky = GetBackgroundColor(D.z * 0.5 + 0.5) * max(dot(N, D), 0.0) * ShadowRayMarch(P + 0.05 * D, D);
        
    return diffuseSun + diffuseSky;
}

float4 main(float2 texCoord : TEXCOORD, float4 pixelCoord : SV_POSITION) : SV_TARGET
{
    hashSeed = float(BaseHash(asuint(pixelCoord.xy))) / float(0xffffffffU) + drawCallParam.floatSeed;
    
    float2 screenSize = perframeResource.screenSize;
    
    float2 planePos = (floor(pixelCoord.xy) + Hash2(hashSeed)) / screenSize * 2.0 - 1.0;
    planePos.x *= screenSize.x / screenSize.y;
    
    float3 cameraPos = float3(cos(drawCallParam.phi) * sin(drawCallParam.theta), cos(drawCallParam.phi) * cos(drawCallParam.theta), sin(drawCallParam.phi)) * drawCallParam.radius;
    
    float3 helper = float3(0.0, 0.0, 1.0);
    
    float3 xVec = normalize(cross(helper, cameraPos));
    float3 yVec = normalize(cross(xVec, cameraPos));
    
    float3 rayDir = normalize(xVec * planePos.x + yVec * planePos.y - normalize(cameraPos) / tan(FOVANGLEY / 2.0));
    float3 curPos = cameraPos;
    
    bool hit = false;
    
    float3 atten = float3(1.0, 1.0, 1.0);
    float3 color = float3(0.0, 0.0, 0.0);
    
    [loop]
    for (uint marchIteration = 0; marchIteration < 150; marchIteration++)
    {
        float distance = SDF(curPos);
        
        curPos += rayDir * distance;
        
        if (distance < LEVELOFDETAIL)
        {
            hit = true;
            break;
        }
    }
    
    if (hit)
    {
        float3 N = GetNormal(curPos);
        
        atten *= materialColor;
        
        rayDir = normalize(N + RandomPointInUnitSphere(hashSeed));
        
        color += atten * ShadeRay(curPos, N, rayDir);
        
        curPos += 0.05 * rayDir;
        
    }
    else
    {
        return float4(float3(0.5, 0.5, 0.5), 1.0 / float(drawCallParam.frameIndex));
    }
    
    [loop]
    for (uint i = 0; i < 4; i++)
    {
        hit = false;
        
        [loop]
        for (uint marchIteration = 0; marchIteration < 50; marchIteration++)
        {
            float distance = SDF(curPos);
            
            curPos += rayDir * distance;
            
            if (distance < LEVELOFDETAIL)
            {
                hit = true;
                break;
            }
        }
    
        if (hit)
        {
            float3 N = GetNormal(curPos);
        
            atten *= materialColor;
        
            rayDir = normalize(N + RandomPointInUnitSphere(hashSeed));
            
            color += atten * ShadeRay(curPos, N, rayDir);
            
            curPos += 0.05 * rayDir;
        }
        else
        {
            break;
        }
    }
    
    return float4(color, 1.0 / drawCallParam.frameIndex);
}

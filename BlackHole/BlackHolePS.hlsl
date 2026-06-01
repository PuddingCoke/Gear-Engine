//修改自https://www.shadertoy.com/view/lstSRS 作者：sonicether

//可平铺噪声来自https://www.shadertoy.com/view/3dVXDc 作者：piyushslayer

#include"Common.hlsli"

#define ITERATIONS 256

cbuffer RenderParameters : register(DRAWCALLCONSTANTS)
{
    uint noiseTextureIndex;
    uint diskTextureIndex;
    float2 iResolution;
    float timeElapsed;
    uint useOriginalVer;
    float texturePeriod;
    float GEXP;
    float exponent1;
    float scale1;
    float bias1;
    float exponent2;
    float scale2;
    float bias2;
    float baseNoise2ScaleFactor;
    float noise2LerpFactor;
}

static Texture2D noiseTexture = ResourceDescriptorHeap[noiseTextureIndex];

static Texture2D diskTexture = ResourceDescriptorHeap[diskTextureIndex];

static const float3 MainColor = float3(1.0, 1.0, 1.0);

static const float2 iMouse = float2(0.0, 0.0);

float pcurve(float x, float a, float b)
{
    float k = pow(a + b, a + b) / (pow(a, a) * pow(b, b));
    return k * pow(x, a) * pow(1.0 - x, b);
}

float sdTorus(float3 p, float2 t)
{
    float2 q = float2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

void Haze(inout float3 color, float3 pos, float alpha)
{
    float2 t = float2(1.0, 0.01);

    float torusDist = length(sdTorus(pos + float3(0.0, -0.05, 0.0), t));

    float bloomDisc = 1.0 / (pow(torusDist, 2.0) + 0.001);
    float3 col = MainColor;
    bloomDisc *= length(pos) < 0.5 ? 0.0 : 1.0;

    color += col * bloomDisc * (2.9 / float(ITERATIONS)) * (1.0 - alpha * 1.0);
}

float3 mod(float3 a, float3 b)
{
    return a - b * floor(a / b);
}

#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uint2(UI0, UI1)
#define UI3 uint3(UI0, UI1, 2798796415U)
#define UIF (1.0 / float(0xffffffffU))

float3 hash33(float3 p)
{
    uint3 q = uint3(int3(p)) * UI3;
    q = (q.x ^ q.y ^ q.z) * UI3;
    return -1. + 2. * float3(q) * UIF;
}

// Gradient noise by iq (modified to be tileable)
float gradientNoise(float3 x, float freq)
{
    // grid
    float3 p = floor(x);
    float3 w = frac(x);
    
    float3 u = w * w * w * (w * (w * 6. - 15.) + 10.);
    
    // gradients
    float3 ga = hash33(mod(p + float3(0., 0., 0.), freq));
    float3 gb = hash33(mod(p + float3(1., 0., 0.), freq));
    float3 gc = hash33(mod(p + float3(0., 1., 0.), freq));
    float3 gd = hash33(mod(p + float3(1., 1., 0.), freq));
    float3 ge = hash33(mod(p + float3(0., 0., 1.), freq));
    float3 gf = hash33(mod(p + float3(1., 0., 1.), freq));
    float3 gg = hash33(mod(p + float3(0., 1., 1.), freq));
    float3 gh = hash33(mod(p + float3(1., 1., 1.), freq));
    
    // projections
    float va = dot(ga, w - float3(0., 0., 0.));
    float vb = dot(gb, w - float3(1., 0., 0.));
    float vc = dot(gc, w - float3(0., 1., 0.));
    float vd = dot(gd, w - float3(1., 1., 0.));
    float ve = dot(ge, w - float3(0., 0., 1.));
    float vf = dot(gf, w - float3(1., 0., 1.));
    float vg = dot(gg, w - float3(0., 1., 1.));
    float vh = dot(gh, w - float3(1., 1., 1.));
	
    // interpolation
    return va +
           u.x * (vb - va) +
           u.y * (vc - va) +
           u.z * (ve - va) +
           u.x * u.y * (va - vb - vc + vd) +
           u.y * u.z * (va - vc - ve + vg) +
           u.z * u.x * (va - vb - ve + vf) +
           u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

float perlin(float3 p, float freq, int octaves, float k)
{
    float G = exp2(GEXP);
    
    float amp = 1.;
    
    float noise = 1.;
    
    for (int i = 0; i < octaves; ++i)
    {
        if (i % 2 == 0)
        {
            p.y += k;
        }
        else
        {
            p.y -= k;
        }
        
        noise *= amp * (gradientNoise(p * freq, freq) * 0.5 + 0.5);
        
        freq *= 2.0;
        
        amp *= G;
    }
    
    return noise;
}

//noise code by iq
float noiseiq(in float3 x)
{
    float3 p = floor(x);
    float3 f = frac(x);
    f = f * f * (3.0 - 2.0 * f);
    float2 uv = (p.xy + float2(37.0, 17.0) * p.z) + f.xy;
    float2 rg = noiseTexture.Sample(linearWrapSampler, (uv + 0.5) / 256.0).yx;
    return -1.0 + 2.0 * lerp(rg.x, rg.y, f.z);
}

void GasDisc(inout float3 color, inout float alpha, float3 pos)
{
    float discRadius = 3.2;
    float discWidth = 5.3;
    float discInner = discRadius - discWidth * 0.5;
    float discOuter = discRadius + discWidth * 0.5;
    
    float3 origin = float3(0.0, 0.0, 0.0);
    float3 discNormal = normalize(float3(0.0, 1.0, 0.0));
    float discThickness = 0.1;

    float distFromCenter = distance(pos, origin);
    float distFromDisc = dot(discNormal, pos - origin);
    
    float radialGradient = 1.0 - saturate((distFromCenter - discInner) / discWidth * 0.5);

    float coverage = pcurve(radialGradient, 4.0, 0.9);

    discThickness *= radialGradient;
    coverage *= saturate(1.0 - abs(distFromDisc) / discThickness);

    float3 dustColorLit = MainColor;
    float3 dustColorDark = float3(0.0, 0.0, 0.0);

    float dustGlow = 1.0 / (pow(1.0 - radialGradient, 2.0) * 290.0 + 0.002);
    float3 dustColor = dustColorLit * dustGlow * 8.2;

    coverage = saturate(coverage * 0.7);


    float fade = pow((abs(distFromCenter - discInner) + 0.4), 4.0) * 0.04;
    float bloomFactor = 1.0 / (pow(distFromDisc, 2.0) * 40.0 + fade + 0.00002);
    float3 b = dustColorLit * pow(bloomFactor, 1.5);
    
    b *= lerp(float3(1.7, 1.1, 1.0), float3(0.5, 0.6, 1.0), pow(radialGradient, 2.0));
    b *= lerp(float3(1.7, 0.5, 0.1), float3(1.0, 1.0, 1.0), pow(radialGradient, 0.5));

    dustColor = lerp(dustColor, b * 150.0, saturate(1.0 - coverage * 1.0));
    coverage = saturate(coverage + bloomFactor * bloomFactor * 0.1);
    
    if (coverage < 0.01)
    {
        return;
    }
    
    float3 radialCoords;
    radialCoords.x = distFromCenter * 1.5 + 0.55;
    radialCoords.y = atan2(-pos.x, -pos.z) * 1.5;
    radialCoords.z = distFromDisc * 1.5;

    radialCoords *= 0.95;
    
    //periodTexture = 1.0 / (speed * 0.5 * 0.15)
    //periodNoise = 1.0 / speed
    //pTex/pNoise= 40/3
    
    float speed = 1.0 / (texturePeriod * 0.5 * 0.15);
    
    float renderPeriod = texturePeriod * 3.0;
    
    float noise1 = 1.0;
    
    float noise2 = 2.0;
    
    if (!useOriginalVer)
    {
        noise1 = 1.0;
        
        float3 rc = radialCoords + 0.0;
        
        noise1 *= perlin(rc, 3.0, 4, fmod(timeElapsed, renderPeriod) * speed);

        noise2 = 2.0;
        
        rc = radialCoords + 30.0;
        
        noise2 *= perlin(rc, 3.0, 6, fmod(timeElapsed, renderPeriod) * speed);
        
        //对比度调节
        noise1 = pow(noise1, exponent1);
        
        noise1 = saturate(noise1 * scale1 - bias1);
        
        float baseNoise2 = noise2;
        
        noise2 = pow(noise2, exponent2);
            
        noise2 = saturate(noise2 * scale2 - bias2);
        
        noise2 = lerp(baseNoise2 * baseNoise2ScaleFactor, noise2, noise2LerpFactor);
    }
    else
    {
        noise1 = 1.0;
        float3 rc = radialCoords + 0.0;
        rc.y += timeElapsed * speed;
        noise1 *= noiseiq(rc * 3.0) * 0.5 + 0.5;
        rc.y -= timeElapsed * speed;
        noise1 *= noiseiq(rc * 6.0) * 0.5 + 0.5;
        rc.y += timeElapsed * speed;
        noise1 *= noiseiq(rc * 12.0) * 0.5 + 0.5;
        rc.y -= timeElapsed * speed;
        noise1 *= noiseiq(rc * 24.0) * 0.5 + 0.5;
        rc.y += timeElapsed * speed;

        noise2 = 2.0;
        rc = radialCoords + 30.0;
        noise2 *= noiseiq(rc * 3.0) * 0.5 + 0.5;
        rc.y += timeElapsed * speed;
        noise2 *= noiseiq(rc * 6.0) * 0.5 + 0.5;
        rc.y -= timeElapsed * speed;
        noise2 *= noiseiq(rc * 12.0) * 0.5 + 0.5;
        rc.y += timeElapsed * speed;
        noise2 *= noiseiq(rc * 24.0) * 0.5 + 0.5;
        rc.y -= timeElapsed * speed;
        noise2 *= noiseiq(rc * 48.0) * 0.5 + 0.5;
        rc.y += timeElapsed * speed;
        noise2 *= noiseiq(rc * 92.0) * 0.5 + 0.5;
        rc.y -= timeElapsed * speed;
    }

    dustColor *= noise1 * 0.998 + 0.002;
    coverage *= noise2;
    
    radialCoords.y += fmod(timeElapsed, renderPeriod) * speed * 0.5;
    
    dustColor *= pow(diskTexture.Sample(linearWrapSampler, radialCoords.yx * float2(0.15, 0.27)).rgb, float3(2.0, 2.0, 2.0)) * 4.0;
    
    coverage = saturate(coverage * 1200.0 / float(ITERATIONS));
    dustColor = max(float3(0.0, 0.0, 0.0), dustColor);

    coverage *= pcurve(radialGradient, 4.0, 0.9);

    color = (1.0 - alpha) * dustColor * coverage + color;

    alpha = (1.0 - alpha) * coverage + alpha;
}

float3 rotate(float3 p, float x, float y, float z)
{
    float3x3 matx = float3x3(1.0, 0.0, 0.0,
                     0.0, cos(x), sin(x),
                     0.0, -sin(x), cos(x));

    float3x3 maty = float3x3(cos(y), 0.0, -sin(y),
                     0.0, 1.0, 0.0,
                     sin(y), 0.0, cos(y));

    float3x3 matz = float3x3(cos(z), sin(z), 0.0,
                     -sin(z), cos(z), 0.0,
                     0.0, 0.0, 1.0);

    p = mul(matx, p);
    p = mul(matz, p);
    p = mul(maty, p);

    return p;
}

void RotateCamera(inout float3 eyevec, inout float3 eyepos)
{
    float mousePosY = iMouse.y / iResolution.y;
    float mousePosX = iMouse.x / iResolution.x;

    float3 angle = float3(mousePosY * 0.05 + 0.05, 1.0 + mousePosX * 1.0, -0.45);

    eyevec = rotate(eyevec, angle.x, angle.y, angle.z);
    eyepos = rotate(eyepos, angle.x, angle.y, angle.z);
}

void WarpSpace(inout float3 eyevec, inout float3 raypos)
{
    float3 origin = float3(0.0, 0.0, 0.0);

    float singularityDist = distance(raypos, origin);
    float warpFactor = 1.0 / (pow(singularityDist, 2.0) + 0.000001);

    float3 singularityVector = normalize(origin - raypos);
    
    float warpAmount = 5.0;

    eyevec = normalize(eyevec + singularityVector * warpFactor * warpAmount / float(ITERATIONS));
}

float rand(float2 coord)
{
    return saturate(frac(sin(dot(coord, float2(12.9898, 78.223))) * 43758.5453));
}

float4 main(float2 uv : TEXCOORD) : SV_TARGET
{
    float aspect = iResolution.x / iResolution.y;

    float2 uveye = uv;
    
    float3 eyevec = normalize(float3((uveye * 2.0 - 1.0) * float2(aspect, 1.0), 6.0));
    float3 eyepos = float3(0.0, -0.0, -10.0);
    
    float2 mousepos = iMouse.xy / iResolution.xy;
    if (mousepos.x == 0.0)
    {
        mousepos.x = 0.35;
    }
    eyepos.x += mousepos.x * 3.0 - 1.5;
    
    const float far = 15.0;

    RotateCamera(eyevec, eyepos);

    float3 color = float3(0.0, 0.0, 0.0);
    
    float dither = rand(uv) * 2.0;

    float alpha = 0.0;
    float3 raypos = eyepos + eyevec * dither * far / float(ITERATIONS);
    
    [loop]
    for (int i = 0; i < ITERATIONS; i++)
    {
        WarpSpace(eyevec, raypos);
        raypos += eyevec * far / float(ITERATIONS);
        GasDisc(color, alpha, raypos);
        Haze(color, raypos, alpha);
    }
    
    color *= 0.02;
    
    color = pow(color, float3(1.5, 1.5, 1.5));
    color = color / (1.0 + color);
    color = pow(color, float3(1.0 / 1.5, 1.0 / 1.5, 1.0 / 1.5));

    
    color = lerp(color, color * color * (3.0 - 2.0 * color), float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.3, 1.20, 1.0));

    color = saturate(color * 1.01);
    
    color = pow(color, float3(0.7 / 2.2, 0.7 / 2.2, 0.7 / 2.2));
    
    return float4(saturate(color), 1.0);
}
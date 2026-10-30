#include"Common.hlsli"

struct PixelInput
{
    float2 texCoord : TEXCOORD;
    float4 lodScales : LODSCALES;
    float3 viewDir : VIEWDIR;
};

cbuffer TextureIndices : register(PER_INVOKE_CONSTANTS)
{
    uint displacement0TextureIndex;
    uint derivative0TextureIndex;
    uint jacobian0TextureIndex;
    uint displacement1TextureIndex;
    uint derivative1TextureIndex;
    uint jacobian1TextureIndex;
    uint displacement2TextureIndex;
    uint derivative2TextureIndex;
    uint jacobian2TextureIndex;
    uint enviromentCubeIndex;
}

cbuffer RenderParam : register(PER_INVOKE_CBUFFER)
{
    float lodScale;
    float lengthScale0;
    float lengthScale1;
    float lengthScale2;
    float sunStrength;
    float sunTheta;
    float specularPower;
    float foamBias;
    float foamScale;
    float sunDirection;
    float exposure;
    float gamma;
}

static Texture2D<float4> displacement0Texture = ResourceDescriptorHeap[displacement0TextureIndex];
static Texture2D<float4> derivative0Texture = ResourceDescriptorHeap[derivative0TextureIndex];
static Texture2D<float> jacobian0Texture = ResourceDescriptorHeap[jacobian0TextureIndex];

static Texture2D<float4> displacement1Texture = ResourceDescriptorHeap[displacement1TextureIndex];
static Texture2D<float4> derivative1Texture = ResourceDescriptorHeap[derivative1TextureIndex];
static Texture2D<float> jacobian1Texture = ResourceDescriptorHeap[jacobian1TextureIndex];

static Texture2D<float4> displacement2Texture = ResourceDescriptorHeap[displacement2TextureIndex];
static Texture2D<float4> derivative2Texture = ResourceDescriptorHeap[derivative2TextureIndex];
static Texture2D<float> jacobian2Texture = ResourceDescriptorHeap[jacobian2TextureIndex];

static TextureCube<float4> enviromentCube = ResourceDescriptorHeap[enviromentCubeIndex];

static const float3 L = normalize(float3(cos(sunTheta) * cos(sunDirection), sin(sunTheta), cos(sunTheta) * sin(sunDirection)));

static const float3 oceanColor = float3(0.0000, 0.2507, 0.3613);

static const float3 sunColor = float3(1.0, 1.0, 1.0);

float3 ACESToneMapping(float3 color)
{
    const float A = 2.51;
    
    const float B = 0.03;
    
    const float C = 2.43;
    
    const float D = 0.59;
    
    const float E = 0.14;

    color *= exposure;
    
    return (color * (A * color + B)) / (color * (C * color + D) + E);
}

float4 main(PixelInput input) : SV_TARGET
{
    float4 derivative = derivative0Texture.Sample(anisotrophicWrapSampler, input.texCoord / lengthScale0);
    
    derivative += derivative1Texture.Sample(anisotrophicWrapSampler, input.texCoord / lengthScale1) * input.lodScales.y;
    
    derivative += derivative2Texture.Sample(anisotrophicWrapSampler, input.texCoord / lengthScale2) * input.lodScales.z;
    
    float J = jacobian0Texture.Sample(anisotrophicWrapSampler, input.texCoord / lengthScale0);
    
    J += jacobian1Texture.Sample(anisotrophicWrapSampler, input.texCoord / lengthScale1);
    
    J += jacobian2Texture.Sample(anisotrophicWrapSampler, input.texCoord / lengthScale2);
    
    float2 slope = float2(derivative.x / (1.0 + derivative.z), derivative.y / (1.0 + derivative.w));
    
    float3 N = normalize(float3(-slope.x, 1.0, -slope.y));
    
    float3 V = normalize(input.viewDir);
    
    if (dot(N, V) < 0.0)
    {
        N = normalize(N - 2.0 * dot(N, V) * V);
    }
    
    float3 R = reflect(-V, N);
    
    float F0 = 0.020018673;
    
    float F = F0 + (1.0 - F0) * pow(1.0 - max(dot(N, V), 0.0), 5.0);
    
    float3 H = normalize(V + L);
    
    float3 skyLight = enviromentCube.Sample(linearWrapSampler, R).rgb;
    
    skyLight = ACESToneMapping(skyLight);
    
    skyLight = pow(skyLight, 1.0 / gamma);
    
    //author:https://github.com/asylum2010
    const float rho = 0.3;
    const float ax = 0.2;
    const float ay = 0.1;

    float3 h = L + V;
    float3 x = cross(L, N);
    float3 y = cross(x, N);

    float mult = (1.0 / (4.0 * 3.1415926535) * rho / (ax * ay * sqrt(max(1e-5, dot(L, N) * dot(V, N)))));
    float hdotx = dot(h, x) / ax;
    float hdoty = dot(h, y) / ay;
    float hdotn = dot(h, N);

    float spec = mult * exp(-((hdotx * hdotx) + (hdoty * hdoty)) / (hdotn * hdotn));
    //
    
    float3 sunLight = spec * sunColor * sunStrength;
    
    float turbulence = min(1.0, max(0.0, (-J + foamBias) * foamScale));
    
    float3 color = (1.0 - turbulence) * ((1.0 - F) * oceanColor + F * (skyLight + sunLight)) + turbulence * float3(0.9, 0.9, 0.9);
    
    return float4(color, 1.0);
}
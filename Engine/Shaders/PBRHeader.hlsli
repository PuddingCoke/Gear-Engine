float GGXDistribution(float3 M, float3 N, float roughness)
{
    //alphaG = r^2 书中的公式
    float alphaG2 = roughness * roughness;
    
    alphaG2 *= alphaG2;
    
    float MdotN = saturate(dot(M, N));
    
    float denominator = MdotN * MdotN * (alphaG2 - 1.0) + 1.0;
    
    denominator = denominator * denominator;
    
    return alphaG2 / denominator;
}

//这个函数使用了优化算法，其结果已经除以了4*NdotL*NdotV
float HeightCorrelatedSmithG_2(float NdotL, float NdotV, float roughness)
{
    float alphaG = roughness * roughness;
    
    float alphaG2 = alphaG * alphaG;
    
    float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
    
    float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
    
    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

float3 FresnelSchlick(float cosTheta, float3 F0, float3 F90)
{
    return F0 + (F90 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
    float energyBias = lerp(0, 0.5, roughness);

    float energyFactor = lerp(1.0, 1.0 / 1.51, roughness);

    float3 fd90 = (energyBias + 2.0 * LdotH * LdotH * roughness).rrr;

    float3 f0 = float3(1.0f, 1.0, 1.0);

    float lightScatter = FresnelSchlick(NdotL, f0, fd90).r;

    float viewScatter = FresnelSchlick(NdotV, f0, fd90).r;

    return lightScatter * viewScatter * energyFactor;
}

//用于解析光源
//输入的所有向量必须为归一化的向量
//F0和Albedo为使用金属度参数化插值后的结果
//roughness为用户控制的感知线性粗糙度
float3 PBR_BRDFEvaluate(float3 N, float3 V, float3 L, float3 F0, float3 Albedo, float roughness)
{
    const float3 H = normalize(L + V);
    
    const float NDF = GGXDistribution(H, N, roughness);
    
    const float3 F = FresnelSchlick(saturate(dot(H, V)), F0, float3(1.0, 1.0, 1.0));
    
    const float NdotV = abs(dot(N, V)) + 1e-5f;
    
    const float NdotL = saturate(dot(N, L));
    
    const float LdotH = saturate(dot(L, H));
    
    const float G = HeightCorrelatedSmithG_2(NdotL, NdotV, roughness);
    
    const float3 specular = F * NDF * G;
    
    const float3 diffuse = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, roughness) * Albedo;
    
    const float PI = 3.14159265358979323846;
    
    return (specular + diffuse) / PI;
}
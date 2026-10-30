struct PixelInput
{
    float4 color : COLOR;
};

float4 main(PixelInput input) : SV_TARGET
{
#ifdef _DEBUG
    return float4(1.0,0.0,0.0,1.0);
#else
    return input.color;
#endif
    
}

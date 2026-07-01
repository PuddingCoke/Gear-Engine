#include"Common.hlsli"

float4 main(float4 pos : POSITION) : SV_POSITION
{
    return perframeResource.deltaTime.rrrr;
}
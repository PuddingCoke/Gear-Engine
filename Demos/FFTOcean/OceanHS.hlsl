struct VS_CONTROL_POINT_OUTPUT
{
    float3 position : POSITION;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 position : POSITION;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, 3> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;

    Output.EdgeTessFactor[0] = 4.0;
    Output.EdgeTessFactor[1] = 4.0;
    Output.EdgeTessFactor[2] = 4.0;

    Output.InsideTessFactor = 4.0;

    return Output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main(
	InputPatch<VS_CONTROL_POINT_OUTPUT, 3> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONTROL_POINT_OUTPUT Output;
    Output.position = ip[i].position;
    return Output;
}

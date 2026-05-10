cbuffer ProjMatrices : register(b3)
{
    matrix viewProj[6];
}

static const float3 vertices[36] =
{
    -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, 1.0f, -1.0f,
             1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,
             1.0f, 1.0f, -1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,
};

struct VertexOutput
{
    float3 worldPos : POSITION;
    float4 position : SV_Position;
    uint renderTargetIndex : SV_RenderTargetArrayIndex;
};

VertexOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    float3 position = vertices[vertexID];
    
    VertexOutput output;
    output.worldPos = position;
    output.position = mul(float4(position, 1.0), viewProj[instanceID]);
    output.renderTargetIndex = instanceID;
    
    return output;
}
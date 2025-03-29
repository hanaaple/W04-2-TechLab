cbuffer BoxCB : register(b0)
{
    float3 Min;
    float _pad0;
    float3 Max;
    float _pad1;
    row_major float4x4 MVP;
}

static const float3 UnitCubeVertices[8] =
{
    float3(0, 0, 0),
    float3(0, 0, 1),
    float3(0, 1, 0),
    float3(0, 1, 1),
    float3(1, 0, 0),
    float3(1, 0, 1),
    float3(1, 1, 0),
    float3(1, 1, 1)
};

struct VS_INPUT
{
    uint vertexId : SV_InstanceID;
};

float4 main(VS_INPUT input) : SV_POSITION
{
    float3 posLocal = lerp(Min, Max, UnitCubeVertices[input.vertexId]);
    return mul(float4(posLocal, 1.0f), MVP);
}
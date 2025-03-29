cbuffer BoxData : register(b0)
{
    row_major float4x4 MVP;
};

float4 main( float3 pos : POSITION ) : SV_POSITION
{
    return mul(float4(pos, 1.0f), MVP);
}
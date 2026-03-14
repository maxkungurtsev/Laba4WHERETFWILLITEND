float4 main(uint id : SV_VertexID) : SV_POSITION
{
    float2 pos[3] =
    {
        float2(-1, -1),
        float2(-1, 3),
        float2(3, -1)
    };

    return float4(pos[id], 0, 1);
}
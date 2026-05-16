struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUT main(uint vertexID : SV_VertexID)
{
    VS_OUT output;

    float2 pos[3] =
    {
        float2(-1.0, -1.0),
        float2(-1.0, 3.0),
        float2(3.0, -1.0)
    };

    float2 uv[3] =
    {
        float2(0.0, 1.0),
        float2(0.0, -1.0),
        float2(2.0, 1.0)
    };

    output.pos = float4(pos[vertexID], 0.0, 1.0);
    output.uv = uv[vertexID];

    return output;
}
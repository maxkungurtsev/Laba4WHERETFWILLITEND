struct VS_IN
{
    float3 pos : POSITION;
};
struct VS_OUT
{
    float4 pos : SV_POSITION;
};
VS_OUT main(VS_IN input)
{
    VS_OUT o;
    o.pos = float4(input.pos, 1);
    return o;
}
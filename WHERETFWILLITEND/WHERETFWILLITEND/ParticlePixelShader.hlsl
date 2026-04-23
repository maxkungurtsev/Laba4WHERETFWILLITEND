Texture2D ParticleTex : register(t1);
SamplerState ParticleSamp : register(s0);

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
    float life : TEXCOORD1;
};
struct PS_OUT
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    int4 material_index : SV_Target2;
};
PS_OUT main(VS_OUT input)
{
    PS_OUT o;
    o.albedo = ParticleTex.Sample(ParticleSamp, input.uv);
    o.normal = float4(0.5, 0.5, 1.0, 1.0);
    o.material_index = int4(0, 0, 0, 0);
    return o;
}
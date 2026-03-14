Texture2D diffuseMap : register(t0);
SamplerState samplerState : register(s0);

cbuffer Material : register(b2)
{
    float time;
    float pad[3];
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PS_OUT
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
};

PS_OUT main(PS_IN input)
{
    PS_OUT gbuffer;
    float2 uv = input.uv + time;
    float4 texColor = diffuseMap.Sample(samplerState, uv);
    float3 N = normalize(input.normal);
    float3 packedNormal = N * 0.5 + 0.5;
    gbuffer.albedo = texColor;
    gbuffer.normal = float4(packedNormal, 1);
    return gbuffer;
}
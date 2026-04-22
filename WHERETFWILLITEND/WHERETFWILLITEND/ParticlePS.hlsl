Texture2D ParticleTex : register(t1);
SamplerState ParticleSamp : register(s0);

struct GS_OUT
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
    float life : TEXCOORD1;
};

float4 main(GS_OUT input) : SV_Target
{
    float4 c = ParticleTex.Sample(ParticleSamp, input.uv);
    return c;
}
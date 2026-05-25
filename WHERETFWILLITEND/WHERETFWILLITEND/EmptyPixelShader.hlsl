Texture2D inputTex : register(t0);
SamplerState Sampler : register(s0);

struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut input) : SV_Target
{
    float4 output = inputTex.Sample(Sampler, input.uv);
    return output;
}
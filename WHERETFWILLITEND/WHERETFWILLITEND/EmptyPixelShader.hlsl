Texture2D inputTex : register(t0);
SamplerState Sample : register(s0);

struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut input) : SV_Target
{
    return inputTex.Sample(Sample, input.uv);
}
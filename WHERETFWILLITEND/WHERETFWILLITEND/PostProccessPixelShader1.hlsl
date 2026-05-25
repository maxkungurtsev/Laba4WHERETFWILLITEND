Texture2D inputTex : register(t0);
Texture2D Normal : register(t1);
SamplerState Sampler : register(s0);

struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut input) : SV_Target
{
    float4 color = inputTex.Sample(Sampler, (1.0f - input.uv));
    float4 normal = Normal.Sample(Sampler, input.uv);
    float4 output = float4(color.x * normal.x, color.y* normal.y, color.z * normal.z, 1.0f);
    return output;
}
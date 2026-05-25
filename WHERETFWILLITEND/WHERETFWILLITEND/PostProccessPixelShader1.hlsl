Texture2D inputTex : register(t0);
SamplerState Sampler : register(s0);
struct shaderMaterialData
{
    float3 ambient_;
    float shiny_;
    float3 diffuse_;
    float NormalType;
    float3 spec_;
    float pad1;
};
cbuffer PassConstants : register(b0)
{
    float4 cam_pos;
    float4 cam_forward;
    shaderMaterialData mats[300];
    float time;
    int current_mat;
    float cam_near;
    float cam_far;
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut input) : SV_Target
{
    float2 uv = input.uv;
    //shift
    
    float r = inputTex.Sample(Sampler, uv + float2(0.004, 0)).r;
    float g = inputTex.Sample(Sampler, uv).g;
    float b = inputTex.Sample(Sampler, uv - float2(0.004, 0)).b;
    float4 basecolor = float4(r,g,b,1.0f);
    float noise =frac(sin(dot(uv * time, float2(12.9898, 78.233))) * 43758.5453);
    float3 color = basecolor.rgb += noise * 0.2;
    bool scanline =frac(sin(dot(uv.y * time, float2(12.9898, 78.233))) * 43758.5453)>0.999f;
    if (scanline)
    {
        color = float3(1.0f, 1.0f, 1.0f);
    }
    float vignette = (0.7 - (abs(uv.x - 0.5f) + abs(uv.y - 0.5f)) / 2.0f);
    color *= vignette;
    float4 output = float4(color, 1.0f);
    return output;
}
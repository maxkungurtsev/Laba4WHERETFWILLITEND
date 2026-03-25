Texture2D diffuseMap : register(t0);
Texture2D NormalMap : register(t1);
SamplerState samplerState : register(s0);

struct LightData
{
    float3 strength;
    float falloff_start;
    float4 direction;
    float4 position;
    float falloff_end;
    float spot_power;
    int type;
    float pad;
};
struct shaderMaterialData
{
    float3 ambient_;
    float shiny_;
    float3 diffuse_;
    float pad0;
    float3 spec_;
    float pad1;
};
cbuffer PassConstants : register(b0)
{
    float4x4 model;
    float4x4 inv_model;
    float4x4 view;
    float4x4 inv_view;
    float4x4 projection;
    float4x4 inv_projection;
    float4 cam_pos;
    float4 cam_forward;
    float3 amb_light;
    float time;
    LightData lights[128];
    shaderMaterialData mats[64];
    float max_lights;
    float current_mat;
    float pad2[2];
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

float4 main(PS_IN input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 finalLight = amb_light;
    for (int j = 0; j < max_lights; j++) {
        float3 L = normalize(lights[j].position.xyz - input.worldPos);
        float3 V = normalize(cam_pos.xyz - input.worldPos);
        float3 diffuse_ = mats[current_mat].diffuse_ * max(dot(N, L), 0);
        float3 R = reflect(-L, N);
        float3 spec = mats[current_mat].spec_ * pow(max(dot(R, V), 0), mats[current_mat].shiny_);
        finalLight += (diffuse_+spec) * lights[j].strength;
    }
    float2 uv = input.uv;
    const int a = current_mat;
    float4 texcolor = diffuseMap.Sample(samplerState, uv);
    float4 FinalColor = float4(texcolor.xyz * finalLight, texcolor.a);
    return FinalColor;
}
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
    float inner_cos;
    float outer_cos;
    float pad[3];
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
    shaderMaterialData mats[300];
    float max_lights;
    int current_mat;
    float pad2[2];
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};
struct PS_OUT
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    int4 material_index : SV_Target2;
};
PS_OUT main(PS_IN input) : SV_TARGET
{
    float2 uv = input.uv;
    PS_OUT output;
    if (max_lights == 0)
    {
        output.albedo = float4(0, 0, 0, 0);
    }
    else
    {
        output.albedo = diffuseMap.Sample(samplerState, uv);
    }
    output.normal = float4(input.normal, 1.0)*0.5+0.5;
    output.material_index = int4(current_mat, 0, 0, 0);
    return output;
}
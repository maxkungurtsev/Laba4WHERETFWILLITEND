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
    shaderMaterialData mats[64];
    float current_mat;
    float pad2[3];
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 tangent : TEXCOORD1;
    float3 bitangent : TEXCOORD2;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    float3 pos = input.pos;
    pos.x += sin(time + pos.y) * 2;
    pos.z += cos(time + pos.x) * 2;
    float4 worldPosition = mul(model, float4(pos, 1.0));
    output.pos = mul(projection, mul(view, worldPosition));
    output.worldPos = worldPosition.xyz;
    output.normal = normalize(mul((float3x3) model, input.normal));
    output.uv = input.uv;
    output.tangent = input.tangent;
    output.bitangent = input.bitangent;
    return output;
}
Texture2D diffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetallicMap : register(t3);
Texture2D AmbientOccolisionMap : register(t4);
SamplerState samplerState : register(s0);

struct shaderMaterialData
{
    float3 ambient_;
    float shiny_;
    float3 diffuse_;
    float NormalType;
    float3 spec_;
    float using_pbr_;
};
cbuffer PassConstants : register(b0)
{
    float4 cam_pos;
    float4 cam_forward;
    float time;
    int current_mat;
    float cam_near;
    float cam_far;
};

cbuffer MaterialConstants : register(b2)
{
    shaderMaterialData mats[300];
};
cbuffer POVConstants : register(b1)
{
    float4x4 model;
    float4x4 inv_model;
    float4x4 view;
    float4x4 inv_view;
    float4x4 projection;
    float4x4 inv_projection;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};
struct PS_OUT
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    int4 material_index : SV_Target2;
    float4 roughness : SV_Target3;
    float4 metallic : SV_Target4;
    float4 ambient_occolision : SV_Target5;
};
PS_OUT main(PS_IN input) : SV_TARGET
{
    float2 uv = input.uv;
    PS_OUT output;
    
    int NormalType = mats[current_mat].NormalType;
    switch (NormalType)
    {
        case 0:{
            output.normal = float4(input.normal, 1.0)*0.5+0.5;
            break; }
        case 1:{
            float3 T = normalize(input.tangent);
            float3 B = normalize(input.bitangent);
            float3 N = normalize(input.normal);
            float3x3 TBN = float3x3(T, B, N);
            float3 normalTex = NormalMap.Sample(samplerState, input.uv).xyz;
            normalTex = normalTex * 2.0 - 1.0;
                output.normal = float4(mul(normalTex, TBN), 1.0) * 0.5 + 0.5;
            break;}
        case 2:{
            float x;
            float y;
            NormalMap.GetDimensions(x, y);
            float dx = 1 / x;
            float dy = 1 / y;
            float hr = NormalMap.Sample(samplerState, input.uv + dx).x;
            float hl = NormalMap.Sample(samplerState, input.uv - dx).x;
            float hu = NormalMap.Sample(samplerState, input.uv + dy).x;
            float hd = NormalMap.Sample(samplerState, input.uv - dy).x;
            float3 T = normalize(input.tangent);
            float3 B = normalize(input.bitangent);
            float3 N = normalize(input.normal);
            float3x3 TBN = float3x3(T, B, N);
            float3 n = float3(hu - hd, 1, hr - hl);
            output.normal = float4(normalize(mul(n, TBN)), 1.0)*0.5+0.5;
            break;}
    }
    output.albedo = diffuseMap.Sample(samplerState, uv).rgba;
    output.roughness = RoughnessMap.Sample(samplerState, uv);
    output.material_index = int4(current_mat, 0, 0, 0);
    output.metallic = MetallicMap.Sample(samplerState, uv);
    output.ambient_occolision = AmbientOccolisionMap.Sample(samplerState, uv);
    return output;
}
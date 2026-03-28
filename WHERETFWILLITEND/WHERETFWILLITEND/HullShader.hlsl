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
    LightData lights[128];
    shaderMaterialData mats[300];
    float max_lights;
    int current_mat;
    float pad2[2];
};

struct HS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};
struct CONTR_POINT_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};
struct CONST_DATA_OUT
{
    float tes_factor[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

[domain("tri")]
[partitioning("fractional_odd")]   // can also be integer / fractional_even / pow2
[outputtopology("triangle_cw")]    // MUST MATCH: your winding/cull mode
[outputcontrolpoints(3)]
[patchconstantfunc("HSConstants")]
[maxtessfactor(64.0)]

CONTR_POINT_OUT main(InputPatch<HS_IN, 3> patch, uint patchID : SV_OutputControlPointID)
{
    CONTR_POINT_OUT output;
    output.pos = patch[patchID].pos;
    output.normal = patch[patchID].normal;
    output.uv = patch[patchID].uv;
    output.worldPos = patch[patchID].worldPos;
    output.tangent = patch[patchID].tangent;
    output.bitangent = patch[patchID].bitangent;
    return output;
}
CONST_DATA_OUT HSConstants(InputPatch<HS_IN, 3> patch){
    CONST_DATA_OUT output;
    float3 p0 = patch[0].worldPos.xyz;
    float3 p1 = patch[1].worldPos.xyz;
    float3 p2 = patch[2].worldPos.xyz;
    
    float3 mid = (p1 + p2) * 0.5f;
    float d = distance(mid, cam_pos.xyz);
    float t = saturate(d / 25.0f);
    t = t * t;
    output.tes_factor[0] =lerp(8.0f, 1.0f, t);
    mid = (p2 + p0) * 0.5f;
    d = distance(mid, cam_pos.xyz);
    t = saturate(d / 25.0f);
    t = t * t;
    output.tes_factor[1] = lerp(8.0f, 1.0f, t);
    mid = (p1 + p0) * 0.5f;
    d = distance(mid, cam_pos.xyz);
    t = saturate(d / 25.0f);
    t = t * t;
    output.tes_factor[2] = lerp(8.0f, 1.0f, t);
    output.inside = (output.tes_factor[0] + output.tes_factor[1] + output.tes_factor[2]) / 3.0f;;
    return output;
}
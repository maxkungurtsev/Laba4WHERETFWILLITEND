Texture2D NormalMap : register(t1);
SamplerState samplerState : register(s0);

cbuffer POVConstants : register(b1)
{
    float4x4 model;
    float4x4 inv_model;
    float4x4 view;
    float4x4 inv_view;
    float4x4 projection;
    float4x4 inv_projection;
};
struct CONTR_POINT_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};
struct CONST_DATA_IN
{
    float tes_factor[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct DS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 bitangent : TEXCOORD3;
};


[domain("tri")]

DS_OUT main(
    CONST_DATA_IN tessFactors, float3 bary : SV_DomainLocation, const OutputPatch<CONTR_POINT_IN, 3> patch)
{
    DS_OUT output;
    output.normal = bary.x * patch[0].normal + bary.y * patch[1].normal + bary.z * patch[2].normal;
    output.normal = normalize(output.normal);
    output.uv = bary.x * patch[0].uv + bary.y * patch[1].uv + bary.z * patch[2].uv;
    output.worldPos = bary.x * patch[0].worldPos + bary.y * patch[1].worldPos + bary.z * patch[2].worldPos;
    output.tangent = bary.x * patch[0].tangent + bary.y * patch[1].tangent + bary.z * patch[2].tangent;
    output.bitangent = bary.x * patch[0].bitangent + bary.y * patch[1].bitangent + bary.z * patch[2].bitangent;
    float disp = NormalMap.SampleLevel(samplerState, output.uv,0).x;
    output.worldPos += output.normal * disp;
    output.pos = mul(projection, mul(view, float4(output.worldPos,1.0)));
    
    return output;
}
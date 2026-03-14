cbuffer CbPerPass : register(b0)
{
    float4x4 model;
    float4x4 inv_model;

    float4x4 view;
    float4x4 inv_view;

    float4x4 projection;
    float4x4 inv_projection;

    float3 cameraPos;
    float pad0;

    float time;
    float nearZ;
    float farZ;
    float pad1;
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 worldPosition = mul(model, float4(input.pos, 1));

    float4 viewPos = mul(view, worldPosition);

    output.pos = mul(projection, viewPos);

    float3x3 normalMatrix = (float3x3) transpose(inv_model);

    output.normal = normalize(mul(normalMatrix, input.normal));

    output.uv = input.uv;

    return output;
}
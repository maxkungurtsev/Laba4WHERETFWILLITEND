cbuffer MVP : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

struct VS_IN {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VS_OUT {
    float4 pos : SV_POSITION; 
    float3 normal : NORMAL;  
    float2 uv : TEXCOORD0;   
    float3 worldPos : TEXCOORD1; 
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    float4 worldPosition = mul(model, float4(input.pos,1.0));
    output.pos = mul(projection, mul(view, worldPosition));
    output.worldPos = worldPosition.xyz;
    output.normal = normalize(mul((float3x3)model, input.normal));
    output.uv = input.uv;
    return output;
}

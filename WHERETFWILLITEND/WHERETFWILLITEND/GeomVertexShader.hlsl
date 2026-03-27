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
    shaderMaterialData mats[300];
    float max_lights;
    int current_mat;
    float pad2[2];
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

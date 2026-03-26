Texture2D diffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D Depth : register(t2);
Texture2D MaterialIndex : register(t3);
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
    shaderMaterialData mats[300];
    float max_lights;
    float current_mat;
    float pad2[2];
};
struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN input) : SV_Target{
    float2 uv = input.uv;
    float3 normal = NormalMap.Sample(samplerState, uv).xyz;
    float3 albedo = diffuseMap.Sample(samplerState, uv).xyz;
    int matIndex = (int) (MaterialIndex.Sample(samplerState, uv).x + 0.5f);

    float depth = Depth.Sample(samplerState, uv).x;
    
    float4 clip = float4(uv * 2 - 1, depth, 1.0);
    float4 viewPos = mul(inv_projection, clip);
    viewPos /= viewPos.w;
    float3 worldPos = viewPos.xyz;
    float3 finalLight = amb_light;
    for (int i = 0; i < max_lights; i++){
        float3 L = normalize(lights[i].position.xyz - worldPos);
        float3 V = normalize(cam_pos.xyz - worldPos);
        float3 diffuse_ = mats[matIndex].diffuse_ * max(dot(normal, L), 0);
        float3 R = reflect(-L, normal);
        float3 spec = mats[matIndex].spec_ * pow(max(dot(R, V), 0), mats[matIndex].shiny_);
        finalLight += (diffuse_+ spec) * lights[i].strength;
    }
    float4 Final = float4(albedo, 1.0);
    return Final;
}
Texture2D diffuseMap : register(t0);
SamplerState samplerState : register(s0);

cbuffer Light : register(b1)
{
    float3 lightPos;
    float pad;
    float3 cameraPos;
    float pad1;
    float4 ambient_k;
    float4 diffuse_k;
    float4 specular_k;
    float shiny_k;
    float intensity;
    float pad3;
    float time;
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
    float3 L = normalize(lightPos - input.worldPos);
    float3 V = normalize(cameraPos - input.worldPos);
    float4 diff = diffuse_k * max(dot(N, L), 0);
    float3 R = reflect(-L, N);
    float4 spec = specular_k * pow(max(dot(R, V), 0), shiny_k);
    float2 uv = input.uv+time;
    float4 texColor = diffuseMap.Sample(samplerState, uv);
    float3 finalRGB = texColor * (ambient_k + diff) + spec +float3(0.1, 0.1, 0.1);
    return float4(finalRGB, texColor.a);
}
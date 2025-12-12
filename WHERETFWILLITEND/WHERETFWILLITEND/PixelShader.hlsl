Texture2D diffuseMap : register(t0);
SamplerState samplerState : register(s0);

cbuffer Light : register(b1)
{
    float3 lightPos;
    float3 cameraPos;
};

struct PS_IN
{
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

float4 main(PS_IN input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 L = normalize(lightPos - input.worldPos);
    float3 V = normalize(cameraPos - input.worldPos);
    float diff = max(dot(N, L), 0);
    float3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0), 32);
    float4 texColor = diffuseMap.Sample(samplerState, input.uv);
    float4 finalColor = texColor * diff + float4(spec, spec, spec, 0);
    return finalColor;
}

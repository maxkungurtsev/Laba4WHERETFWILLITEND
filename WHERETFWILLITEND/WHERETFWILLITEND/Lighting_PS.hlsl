Texture2D gAlbedo : register(t0);
Texture2D gNormal : register(t1);
Texture2D gDepth : register(t2);

SamplerState samp : register(s0);

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

cbuffer CbLight : register(b1)
{
    float3 lightPos;
    float pad;

    float3 lightDir;
    float lightType;

    float4 ambient_k;
    float4 diffuse_k;
    float4 specular_k;

    float shiny_k;
    float intensity;
    float range;
    float spotCutoff;

    float2 screenSize;
    float pad2[2];
};

float3 ReconstructWorldPos(float depth, float2 uv)
{
    float4 clip;

    clip.xy = uv * 2 - 1;
    clip.z = depth;
    clip.w = 1;

    float4 viewPos = mul(inv_projection, clip);
    viewPos /= viewPos.w;

    float4 worldPos = mul(inv_view, viewPos);

    return worldPos.xyz;
}

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
    float2 uv = pos.xy / screenSize;

    float4 albedo = gAlbedo.Sample(samp, uv);

    float3 N = gNormal.Sample(samp, uv).xyz * 2 - 1;

    float depth = gDepth.Sample(samp, uv).r;

    float3 worldPos = ReconstructWorldPos(depth, uv);

    float3 L = normalize(lightPos - worldPos);
    float3 V = normalize(cameraPos - worldPos);

    float diff = max(dot(N, L), 0);

    float3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0), shiny_k);

    float3 color =
        albedo.rgb * (ambient_k.rgb + diff * diffuse_k.rgb) +
        spec * specular_k.rgb;

    return float4(color, 1);
}
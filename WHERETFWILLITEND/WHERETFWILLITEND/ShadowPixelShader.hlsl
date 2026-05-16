Texture2D Depth : register(t0);
SamplerState samplerState : register(s0);

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
    shaderMaterialData mats[300];
    float time;
    int current_mat;
    float pad2[2];
};

cbuffer ShadowViewProj : register(b1)
{
    float4x4 shadow_view_proj;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float RestoreCameraDepth(int2 pix)
{
    return Depth.Load(int3(pix, 0)).x;
}

float3 RestoreWorldPosition(float2 uv, float depth)
{
    float2 ndc;
    ndc.x = uv.x * 2.0 - 1.0;
    ndc.y = 1.0 - uv.y * 2.0;

    float4 clip = float4(ndc, depth, 1.0);
    float4 viewPos = mul(inv_projection, clip);
    viewPos /= viewPos.w;

    return mul(inv_view, viewPos).xyz;
}

float main(PS_IN input) : SV_Depth
{
    uint w, h;
    Depth.GetDimensions(w, h);

    int2 pix = int2(input.pos.xy);
    float2 uv = (pix + 0.5) / float2(w, h);
    float depth = RestoreCameraDepth(pix);

    if (depth >= 1.0)
    {
        discard;
    }

    float3 worldPos = RestoreWorldPosition(uv, depth);
    float4 shadowClip = mul(shadow_view_proj, float4(worldPos, 1.0));
    shadowClip.xyz /= shadowClip.w;

    return saturate(shadowClip.z);
}
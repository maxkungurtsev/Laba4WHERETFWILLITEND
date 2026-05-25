Texture2D inputTex : register(t0);
Texture2D Depth : register(t1);
Texture2D Normal : register(t2);
SamplerState Sampler : register(s0);
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
    float4 cam_pos;
    float4 cam_forward;
    shaderMaterialData mats[300];
    float time;
    int current_mat;
    float cam_near;
    float cam_far;
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut input) : SV_Target
{
    float4 output = inputTex.Sample(Sampler, input.uv);
    bool is_edge = false;
    float edge_bias = 0.0001f;
    float edge_radius = 0.00007f;
    float z = cam_near * cam_far / (cam_far - Depth.Sample(Sampler, input.uv).r * (cam_far - cam_near)) / cam_far;
    float4 norm = normalize(Normal.Sample(Sampler, input.uv));
    for (int i=-10; i < 10; i++)
    {
        for (int j=-10; j < 10; j++)
        {
            float2 close_uv = float2(input.uv.x + i*edge_radius, input.uv.y + j * edge_radius);
            close_uv.x = max(close_uv.x, 0.0f);
            close_uv.x = min(close_uv.x, 1.0f);
            close_uv.y = max(close_uv.y, 0.0f);
            close_uv.y = min(close_uv.y, 1.0f); 
            float z_near = cam_near * cam_far / (cam_far - Depth.Sample(Sampler, close_uv).r * (cam_far - cam_near)) / cam_far;
            float4 norm_near = normalize(Normal.Sample(Sampler, close_uv));
            float dot_norm = dot(norm, norm_near);
            if ((abs(z - z_near) > edge_bias && dot_norm < 0.99f))
            {
                return float4(0.0f, 0.0f, 0.0f, 1.0f);
            }

        }
    }
    return output;
}
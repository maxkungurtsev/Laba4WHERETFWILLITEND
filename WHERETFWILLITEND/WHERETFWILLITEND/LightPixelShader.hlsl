Texture2D diffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D Depth : register(t2);
Texture2D MaterialIndex : register(t3);
SamplerState samplerState : register(s0);
Texture2D shadowMaps[4] : register(t5);
struct LightData
{
    float3 strength;
    float falloff_start;
    float4 direction;
    float4 position;
    float falloff_end;
    float spot_power;
    int type;
    float velocity;
    float spawn_time;
    float pad[3];
    float4 movement_direction;
};

StructuredBuffer<LightData> lights : register(t4);

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
cbuffer POVConstants : register(b1)
{
    float4x4 model;
    float4x4 inv_model;
    float4x4 view;
    float4x4 inv_view;
    float4x4 projection;
    float4x4 inv_projection;

};
cbuffer MaxLights : register(b2)
{
    float4 max_lights;
}
cbuffer ShadowConstants : register(b3)
{
    float4x4 view_proj_mat[4];
    float4 cascade_split_depths;
}

float CalcShadowFactor(float3 worldPos, float viewDepth)
{
    const float depthBias = 0.0015f;
    int cascade = 3;
    for (int j = 0; j < 4; ++j)
    {
        if (viewDepth <= cascade_split_depths[j]){
            cascade = j;
            break;
        }
    }
    float4 shadowClip;
    switch (cascade)
    {
        case 0:
            shadowClip = mul(view_proj_mat[0], float4(worldPos, 1.0f));
            //shadowClip = mul(float4(worldPos, 1.0f),view_proj_mat[0]);
            break;
        case 1:
            shadowClip = mul(view_proj_mat[1], float4(worldPos, 1.0f));
            //shadowClip = mul(float4(worldPos, 1.0f),view_proj_mat[1]);
            break;
        case 2:
            shadowClip = mul(view_proj_mat[2], float4(worldPos, 1.0f));
            //shadowClip = mul(float4(worldPos, 1.0f),view_proj_mat[2]);
            break;
        case 3:
            shadowClip = mul(view_proj_mat[3], float4(worldPos, 1.0f));
            //shadowClip = mul(float4(worldPos, 1.0f),view_proj_mat[3]);
            break;
    }
    shadowClip.xyz /= shadowClip.w;

    float2 shadowUv = float2(shadowClip.x * 0.5f + 0.5f, 0.5f - shadowClip.y * 0.5f);
    bool insideShadowMap = shadowUv.x >= 0.0f && shadowUv.x <= 1.0f &&
                           shadowUv.y >= 0.0f && shadowUv.y <= 1.0f &&
                           shadowClip.z >= 0.0f && shadowClip.z <= 1.0f;

    if (!insideShadowMap)
    {
        return 1.0f;
    }
    float storedDepth;
    switch (cascade)
    {
        case 0:
            storedDepth = shadowMaps[0].Sample(samplerState, shadowUv).r;
            break;
        case 1:
            storedDepth = shadowMaps[1].Sample(samplerState, shadowUv).r;
            break;
        case 2:
            storedDepth = shadowMaps[2].Sample(samplerState, shadowUv).r;
            break;
        case 3:
            storedDepth = shadowMaps[3].Sample(samplerState, shadowUv).r;
            break;
    }
    return (shadowClip-depthBias > storedDepth) ? 0.5f : 1.0f;
}

float3 CalcLight(LightData light, float3 normal, float3 worldPos, float3 viewDir, shaderMaterialData mat)
{
    float3 lightContrib = 0.0f;
    float4 pos = light.position;
    if (light.velocity > 0.0f){
        pos += normalize(light.movement_direction) * light.velocity * (time - light.spawn_time);
    }
        switch (light.type)
        {
            case 0: // directional
        {
                    float3 L = normalize(-light.direction.xyz); // если direction = "куда свет светит"
                    float NdotL = max(dot(normal, L), 0.0f);

                    float3 diffuse = mat.diffuse_ * NdotL;

                    float3 R = normalize(reflect(-L, normal));
                    float specPow = pow(max(dot(R, viewDir), 0.00001), mat.shiny_);
                    float3 spec = mat.spec_ * specPow;

                    lightContrib = (diffuse + spec) * light.strength;
                    break;
                }

            case 1: // point
        {
                    float3 toLight = pos.xyz - worldPos;
                    float dist = length(toLight);
                    float3 L = normalize(pos.xyz - worldPos);
                    float NdotL = max(dot(normal, L), 0.0f);

                    float attenuation = 1.0f;
                    float range = max(light.falloff_end - light.falloff_start, 0.00001f);

                    attenuation = saturate((light.falloff_end - dist) / range);

                    float3 diffuse = mat.diffuse_ * NdotL;

                    float3 R = normalize(reflect(-L, normal));
                    float specPow = pow(max(dot(R, viewDir), 0.000000001), mat.shiny_);
                    float3 spec = mat.spec_ * specPow;

                    lightContrib = (diffuse + spec) * light.strength * attenuation;
                    break;
                }

            case 2: // spot
        {
                    float3 L = normalize(pos.xyz - worldPos);
                    float NdotL = max(dot(normal, L), 0.0f);
                    float3 spotDir = normalize(-light.direction.xyz); // ось прожектор
                    float spotCos = dot(L, spotDir);

                    float spotFactor = smoothstep(light.falloff_end, light.falloff_start, spotCos);
                    spotFactor = pow(spotFactor, light.spot_power);
                    spotFactor = saturate(spotFactor);
                    float3 diffuse = mat.diffuse_ * NdotL;

                    float3 R = normalize(reflect(-L, normal));
                    float specPow = pow(max(dot(R, viewDir), 0.000000001), mat.shiny_);
                    float3 spec = mat.spec_ * specPow;

                    lightContrib = (diffuse + spec) * light.strength * spotFactor;
                    break;
                }
            case 3: // ambient
        {
                    lightContrib = light.strength;
                    break;
                }
        }

        return lightContrib;
    }


struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN input) : SV_Target{
    uint w, h;
    Depth.GetDimensions(w, h);
    int2 pix = int2(input.pos.xy); // screen pixel
    float2 uv = (pix + 0.5) / float2(w, h); // for albedo/normal if needed
    
    float3 albedo = diffuseMap.Sample(samplerState, uv).xyz;
    float3 normal = normalize(NormalMap.Sample(samplerState, uv).xyz*2 -1);


    float depth = Depth.Load(int3(pix, 0)).x; // no filtering
    int matIndex = MaterialIndex.Load(int3(pix, 0)).x;

    float2 ndc;
    ndc.x = uv.x * 2.0 - 1.0;
    ndc.y = 1.0 - uv.y * 2.0; // y flip for D3D screen->NDC

    float4 clip = float4(ndc, depth, 1.0);
    float4 viewPos = mul(inv_projection, clip);
    viewPos /= viewPos.w;
    float3 worldPos = mul(inv_view, viewPos).xyz;
    float3 finalLight = float3(0, 0, 0);
    float3 V = normalize(cam_pos.xyz - worldPos);
    uint elementCount;
    uint stride;
    lights.GetDimensions(elementCount, stride);
    float shadowFactor = 1.0f;
    float viewDepth = abs(viewPos.z);
    for (int i = 0; i < max_lights.x; i++)
    {
        if (lights[i].type == 0)
        {
            shadowFactor = CalcShadowFactor(worldPos, viewDepth);

        }
        finalLight += CalcLight(lights[i], normal, worldPos, V, mats[matIndex])*shadowFactor;
    }
    float4 Final;
    Final = float4(albedo * finalLight, 1.0);
    float storedDepth1 = Depth.Sample(samplerState, uv).r;
    float storedDepth2 = shadowFactor;
    float z2 = ((storedDepth2 - cam_near) / (cam_far - cam_near));
    float z = cam_near * cam_far / (cam_far - storedDepth2 * (cam_far - cam_near)) / cam_far;
    //float d = shadowMaps[1].Sample(samplerState, uv).r;
    return Final;
    return float4(shadowFactor, shadowFactor, shadowFactor, 1.0f);
    if (z < 0)
    {
        return float4(0.0f, 0.7f, 0.0f, 1.0f);
    }
    else if (z > 1)
    {
        return float4(0.7f, 0.0f, 0.0f, 1.0f);
    }
    //return float4(finalLight, 1.0f);
}
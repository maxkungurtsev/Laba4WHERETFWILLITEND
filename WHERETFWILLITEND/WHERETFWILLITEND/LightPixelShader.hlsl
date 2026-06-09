Texture2D diffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D Depth : register(t2);
Texture2D MaterialIndex : register(t3);
Texture2D RoughnessMap : register(t4);
Texture2D MetallicMap : register(t5);
Texture2D AOMap : register(t6);
SamplerState samplerState : register(s0);
Texture2D shadowMap0 : register(t8);
Texture2D shadowMap1 : register(t9);
Texture2D shadowMap2 : register(t10);
Texture2D shadowMap3 : register(t11);
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

StructuredBuffer<LightData> lights : register(t7);

struct shaderMaterialData
{
    float3 ambient_;
    float shiny_;
    float3 diffuse_;
    float NormalType;
    float3 spec_;
    float using_pbr_;
};
cbuffer PassConstants : register(b0)
{
    float4 cam_pos;
    float4 cam_forward;
    float time;
    int current_mat;
    float cam_near;
    float cam_far;
};

cbuffer MaterialConstants : register(b4)
{
    shaderMaterialData mats[300];
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
    float4x4 shad_view[4];
    float4x4 shad_proj[4];
    float4 cascade_split_depths;
}

static const float PI = 3.14159265359f;

float4 CalcShadowFactor(float3 worldPos, float viewDepth)
{
    const float depthBias = 0.01f;
    const float PCF_sample_radius=0.001f;
    int cascade = 3;
    for (int j = 0; j < 4; ++j)
    {
        if (viewDepth < cascade_split_depths[j]){
            cascade = j;
            break;
        }
    }
    float4 shadowClip;
    switch (cascade)
    {
        case 0:
            //shadowClip = mul(view_proj_mat[0], float4(worldPos, 1.0f));
            shadowClip = mul(shad_proj[0], mul(shad_view[0], float4(worldPos, 1.0f)));
            break;
        case 1:
            //shadowClip = mul(view_proj_mat[1], float4(worldPos, 1.0f));
            shadowClip = mul(shad_proj[1], mul(shad_view[1], float4(worldPos, 1.0f)));
            break;
        case 2:
            //shadowClip = mul(view_proj_mat[2], float4(worldPos, 1.0f));
            shadowClip = mul(shad_proj[2], mul(shad_view[2], float4(worldPos, 1.0f)));
            break;
        case 3:
            //shadowClip = mul(view_proj_mat[3], float4(worldPos, 1.0f));
            shadowClip = mul(shad_proj[3], mul(shad_view[3], float4(worldPos, 1.0f)));
            break;
    }
    shadowClip /= shadowClip.w;
    float2 shadowUv = float2(shadowClip.x * 0.5f + 0.5f, 0.5f - shadowClip.y * 0.5f);
    bool insideShadowMap = shadowUv.x >= 0.0f && shadowUv.x <= 1.0f &&
                           shadowUv.y >= 0.0f && shadowUv.y <= 1.0f &&
                           shadowClip.z >= 0.0f && shadowClip.z <= 1.0f;
    if (!insideShadowMap)
    {
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    float storedDepth;
    float4 accumulated_light = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float iterations=0.0f;
    for (int i = -2; i < 2; i++)
    {
        for (int j = -2; j < 2; j++)
        {
            float2 sampleUV = shadowUv + (float2(i, j) * PCF_sample_radius/ (cascade+1));
            sampleUV.x = max(sampleUV.x, 0.0f);
            sampleUV.x = min(sampleUV.x, 1.0f);
            sampleUV.y = max(sampleUV.y, 0.0f);
            sampleUV.y = min(sampleUV.y, 1.0f);
            switch (cascade)
            {
                case 0:
                   storedDepth = shadowMap0.Sample(samplerState, sampleUV).r;
                    break;
                case 1:
                    storedDepth = shadowMap1.Sample(samplerState, sampleUV).r;
                    break;
                case 2:
                    storedDepth = shadowMap2.Sample(samplerState, sampleUV).r;
                    break;
                case 3:
                    storedDepth = shadowMap3.Sample(samplerState, sampleUV).r;
                    break;
            }
            if (shadowClip.z - depthBias / (cascade + 1) > storedDepth)
            {
                switch (cascade)
                {
                    case 0:
                        accumulated_light += float4(1.0f, 0.0f, 0.0f, 1.0f);
                        break;
                    case 1:
                        accumulated_light += float4(0.0f, 1.0f, 0.0f, 1.0f);
                        break;
                    case 2:
                        accumulated_light += float4(0.0f, 0.0f, 1.0f, 1.0f);
                        break;
                    case 3:
                        accumulated_light += float4(10000.0f, 10000.0f, 10000.0f, 100000.0f);
                        break;
                }
                accumulated_light += 0.3f;
            }
            else
            {
                accumulated_light += float4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            iterations++;
        }
    }
    return accumulated_light/iterations;
}


float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
    return a2 / max(denom, 1e-6f);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return NdotV / max(NdotV * (1.0f - k) + k, 1e-6f);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggxV = GeometrySchlickGGX(NdotV, roughness);
    float ggxL = GeometrySchlickGGX(NdotL, roughness);

    return ggxV * ggxL;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 CalcLightPBR(LightData light, float3 normal, float3 worldPos, float3 view_dir, float3 albedo, float roughness, float metallic, float ao)
{
    float4 pos = light.position;
    if (light.velocity > 0.0f)
    {
        pos += normalize(light.movement_direction) * light.velocity * (time - light.spawn_time);
    }

    roughness = clamp(roughness, 0.04f, 1.0f);
    metallic = saturate(metallic);
    ao = saturate(ao);

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

    float3 light_pos = 0.0f;
    float3 radiance = light.strength;
    float attenuation = 1.0f;
    float spotFactor = 1.0f;

    switch (light.type)
    {
        case 0: // directional
        {
                light_pos = normalize(-light.direction.xyz);
                break;
            }

        case 1: // point
        {
                float3 toLight = pos.xyz - worldPos;
                float dist = length(toLight);
                light_pos = normalize(toLight);

                float range = max(light.falloff_end - light.falloff_start, 0.00001f);
                attenuation = saturate((light.falloff_end - dist) / range);
                break;
            }

        case 2: // spot
        {
                float3 toLight = pos.xyz - worldPos;
                float dist = length(toLight);
                light_pos = normalize(toLight);

                float range = max(light.falloff_end - light.falloff_start, 0.00001f);
                attenuation = saturate((light.falloff_end - dist) / range);

                float3 spotDir = normalize(-light.direction.xyz);
                float spotCos = dot(light_pos, spotDir);

                spotFactor = smoothstep(light.falloff_end, light.falloff_start, spotCos);
                spotFactor = pow(spotFactor, light.spot_power);
                spotFactor = saturate(spotFactor);
                break;
            }

        case 3: // ambient fallback
        {
                return light.strength * ao;
            }
    }

    float NdotL = max(dot(normal, light_pos), 0.0f);
    float NdotV = max(dot(normal, view_dir), 0.0f);

    if (NdotL <= 0.0f || NdotV <= 0.0f)
        return 0.0f;

    float3 H = normalize(view_dir + light_pos);

    float3 F = FresnelSchlick(max(dot(H, view_dir), 0.0f), F0);
    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, view_dir, light_pos, roughness);

    float3 numerator = NDF * G * F;
    float denom = max(4.0f * NdotV * NdotL, 1e-4f);
    float3 specular = numerator / denom;

    float3 kS = F;
    float3 kD = (1.0f - kS) * (1.0f - metallic);
    float3 diffuse = kD;

    float3 Lo = (diffuse + specular) * radiance * NdotL * attenuation * spotFactor;

    return Lo;
}

float3 CalcLightPhong(LightData light, float3 normal, float3 worldPos, float3 viewDir, shaderMaterialData mat)
{
    float3 lightContrib = 0.0f;
    float4 pos = light.position;
    if (light.velocity > 0.0f)
    {
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

float3 CalcLight(LightData light, float3 normal, float3 worldPos, float3 viewDir, shaderMaterialData mat, float3 albedo, float roughness, float metallic, float ao)
{
    if (mat.using_pbr_)
    {
        return CalcLightPBR(light, normal, worldPos, viewDir, albedo, roughness, metallic, ao);
    }
    else
    {
        return CalcLightPhong(light, normal, worldPos, viewDir, mat);
    }
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
    float4 shadowFactor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float viewDepth = abs(viewPos.z);
    
    int cascade = 3;
    for (int j = 0; j < 4; ++j)
    {
        if (viewDepth < cascade_split_depths[j])
        {
            cascade = j;
            break;
        }
    }
    
    
        
    for (int i = 0; i < max_lights.x; i++)
    {
        
        shadowFactor = 1.0f;
        float3 light = CalcLight(lights[i], normal, worldPos, V, mats[matIndex], albedo, (RoughnessMap.Sample(samplerState, uv).r), MetallicMap.Sample(samplerState, uv).r, AOMap.Sample(samplerState, uv).r);
        if (lights[i].type == 0)
        {
           // shadowFactor = CalcShadowFactor(worldPos, viewDepth);
        }
        float4 out_light = float4(light.x * shadowFactor.x, light.y * shadowFactor.y, light.z * shadowFactor.z, 1.0f);
        finalLight += out_light;
    }
    float4 Final;
    Final = float4(albedo*finalLight, 1.0);
    return Final;
    float storedDepth1 = Depth.Sample(samplerState, uv).r;
    float storedDepth2 = shadowMap0.Sample(samplerState, uv).r;
    float z2 = ((storedDepth2 - cam_near) / (cam_far - cam_near));
    float z = cam_near * cam_far / (cam_far - storedDepth2 * (cam_far - cam_near)) / cam_far;
    //return float4(shadowFactor, shadowFactor, shadowFactor, 1.0f);
    
}
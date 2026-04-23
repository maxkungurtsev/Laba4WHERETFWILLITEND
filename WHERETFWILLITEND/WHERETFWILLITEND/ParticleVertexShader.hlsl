struct Particle
{
    float4 position;
    float3 velocity;
    float remaining_life;
};
StructuredBuffer<Particle> Particles : register(t0);

cbuffer ParticleRenderCB : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float particleSize;
    uint aliveCount;
    float2 pad0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
    float life : TEXCOORD1;
};

VS_OUT main(uint vertexID : SV_VertexID)
{
    VS_OUT o;
    uint particleId = vertexID / 6;
    uint cornerId = vertexID % 6;

    if (particleId >= aliveCount)
    {
        o.posH = float4(2.0, 2.0, 1.0, 1.0);
        o.uv = float2(0.0, 0.0);
        o.life = 0.0;
        return o;
    }

    Particle p = Particles[particleId];

    static const float2 kUV[6] =
    {
        float2(0.0, 1.0),
        float2(1.0, 1.0),
        float2(0.0, 0.0),
        float2(0.0, 0.0),
        float2(1.0, 1.0),
        float2(1.0, 0.0)
    };

    static const float2 kOffsets[6] =
    {
        float2(-1.0, -1.0),
        float2(1.0, -1.0),
        float2(-1.0, 1.0),
        float2(-1.0, 1.0),
        float2(1.0, -1.0),
        float2(1.0, 1.0)
    };

    float3 worldPos = p.position.xyz;

    float3 right = float3(view[0][0], view[1][0], view[2][0]);
    float3 upv = float3(view[0][1], view[1][1], view[2][1]);

    float halfSize = particleSize * 0.5;
    float2 offset = kOffsets[cornerId] * halfSize;

    float3 worldCorner = worldPos + right * offset.x + upv * offset.y;
    float4 viewCorner = mul(view, float4(worldCorner, 1.0));

    o.posH = mul(projection, viewCorner);
    o.uv = kUV[cornerId];
    o.life = p.remaining_life;
    return o;
}
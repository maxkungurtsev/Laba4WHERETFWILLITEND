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
    if (p.remaining_life <= 0.0)
    {
        o.posH = float4(2.0, 2.0, 1.0, 1.0);
        o.uv = float2(0.0, 0.0);
        o.life = 0.0;
        return o;
    }
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

    // Camera-facing billboard axes in world space come from VIEW matrix columns
    // (for our mul(view, position) convention). Using rows makes quads skew/vanish.

    float halfSize = particleSize * 0.5;
    float2 offset = kOffsets[cornerId] * halfSize;

    float4 viewCenter = mul(view, float4(worldPos, 1.0));
    float4 viewCorner = viewCenter + float4(offset.x, offset.y, 0.0, 0.0);

    o.posH = mul(projection, viewCorner);
    o.uv = kUV[cornerId];
    o.life = p.remaining_life;
    return o;
}
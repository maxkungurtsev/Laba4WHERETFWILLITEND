struct GS_OUT
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
    float life : TEXCOORD1;
};
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
    uint id : PARTICLE_ID;
};
[maxvertexcount(4)]
void main(point VS_OUT input[1], inout TriangleStream<GS_OUT> triStream)
{
    uint id = input[0].id;
    if (id >= aliveCount)
        return;

    Particle p = Particles[id];

    float3 worldPos = p.position.xyz;

    // billboard basis from view matrix rows (camera right/up)
    float3 right = float3(view[0][0], view[1][0], view[2][0]);
    float3 upv = float3(view[0][1], view[1][1], view[2][1]);

    float halfSize = particleSize * 0.5;

    float3 w0 = worldPos + (-right - upv) * halfSize;
    float3 w1 = worldPos + (right - upv) * halfSize;
    float3 w2 = worldPos + (-right + upv) * halfSize;
    float3 w3 = worldPos + (right + upv) * halfSize;

    float4 v0 = mul(view, float4(w0, 1.0));
    float4 v1 = mul(view, float4(w1, 1.0));
    float4 v2 = mul(view, float4(w2, 1.0));
    float4 v3 = mul(view, float4(w3, 1.0));

    GS_OUT o;

    o.life = p.remaining_life;

    o.posH = mul(projection, v0);
    o.uv = float2(0, 1);
    triStream.Append(o);
    o.posH = mul(projection, v1);
    o.uv = float2(1, 1);
    triStream.Append(o);
    o.posH = mul(projection, v2);
    o.uv = float2(0, 0);
    triStream.Append(o);
    o.posH = mul(projection, v3);
    o.uv = float2(1, 0);
    triStream.Append(o);
}
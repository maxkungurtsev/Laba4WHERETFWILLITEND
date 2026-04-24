struct Particle
{
    float4 position;
    float3 velocity;
    float remaining_life;
};

cbuffer ParticleSimCB : register(b0)
{
    float4 emitterPos;
    float dt;
    float time;
    uint aliveInCount;
    uint deadInCount;
    uint emitCount;
    float pad[3];
};

// NOTE:
// Use ping-pong resources on CPU side:
//   AliveIn  -> consume from previous alive buffer
//   AliveOut -> append to next alive buffer
//   DeadIn   -> consume from previous dead buffer
//   DeadOut  -> append to next dead buffer
ConsumeStructuredBuffer<Particle> AliveIn : register(u0);
AppendStructuredBuffer<Particle> AliveOut : register(u1);
ConsumeStructuredBuffer<Particle> DeadIn : register(u2);
AppendStructuredBuffer<Particle> DeadOut : register(u3);

float Hash11(float n)
{
    return frac(sin(n) * 43758.5453123);
}

float3 RandomDirInCone(uint seed, float3 axis, float coneAngle)
{
    float s = (float) seed;

    // ƒве случайные величины [0,1]
    float u1 = Hash11(s * 13.37);
    float u2 = Hash11(s * 17.11);

    // –авномерное распределение внутри конуса
    float cosTheta = lerp(cos(coneAngle), 1.0, u1);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = u2 * 6.28318530718;

    // Ћокальное направление (ось конуса = +Z)
    float3 localDir = float3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );

    // ѕостроение ортонормированного базиса вокруг axis
    axis = normalize(axis);

    float3 up = (abs(axis.z) < 0.999f)
        ? float3(0.0f, 0.0f, 1.0f)
        : float3(1.0f, 0.0f, 0.0f);

    float3 right = normalize(cross(up, axis));
    float3 forward = cross(axis, right);

    // ѕоворот из локального пространства в мировое
    return normalize(
        localDir.x * right +
        localDir.y * forward +
        localDir.z * axis
    );
}

[numthreads(128, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;

    // 1) Simulate currently alive particles
    if (i < aliveInCount)
    {
        Particle p = AliveIn.Consume();
        p.remaining_life -= dt;
        if (p.remaining_life > 0.0)
        {
            int drag = 10;
            p.velocity *= max(0.0, 1.0 - drag * dt);
            p.position.xyz += p.velocity * dt;

            AliveOut.Append(p);
        }
        else
        {
            DeadOut.Append(p);
        }
    }

    // 2) Spawn new particles from dead pool
    if (i < deadInCount)
    {
        Particle p = DeadIn.Consume();

        if (i < emitCount)
        {
            float s = (float) (i + aliveInCount + asuint(time));
            float lifeRnd = Hash11(s * 3.1);
            float spdRnd = Hash11(s * 7.7);

            p.position = emitterPos;
            int lifeMin = 1;
            int lifeMax = 3;
            p.remaining_life = lerp(lifeMin, lifeMax, lifeRnd);
            int speedMin = 100;
            int speedMax = 200;
            float speed = lerp(speedMin, speedMax, spdRnd);
            float3 emiterdir = float3(0, 1, 0);
            float3 dir = RandomDirInCone(i + asuint(time * 1000.0), emiterdir, 5.0f);
            p.velocity = dir * speed;
            AliveOut.Append(p);
        }
        else
        {
            DeadOut.Append(p);
        }
    }
}
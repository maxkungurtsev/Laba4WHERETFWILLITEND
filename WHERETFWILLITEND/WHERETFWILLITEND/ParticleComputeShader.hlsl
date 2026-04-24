struct Particle
{
    float4 position;
    float3 velocity;
    float remaining_life;
};

cbuffer ParticleSimCB : register(b0)
{
    float dt;
    float time;
    uint aliveInCount;
    uint deadInCount; 
    uint emitCount;
    float4 emitterPos;
    float pad[2];
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

float3 RandomDir(uint seed)
{
    float s = (float) seed;
    float a = Hash11(s * 13.37) * 6.2831853;
    float z = Hash11(s * 17.11) * 2.0 - 1.0;
    float r = sqrt(saturate(1.0 - z * z));
    float3 rnd = float3(r * cos(a), z, r * sin(a));
    return rnd;
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
            int lifeMin = 5;
            int lifeMax = 15;
            p.remaining_life = lerp(lifeMin, lifeMax, lifeRnd);
            int speedMin = 10;
            int speedMax = 100;
            float speed = lerp(speedMin, speedMax, spdRnd);
            float3 dir = RandomDir(i + asuint(time * 1000.0));
            p.velocity = dir * speed;
            AliveOut.Append(p);
        }
        else
        {
            DeadOut.Append(p);
        }
    }
}
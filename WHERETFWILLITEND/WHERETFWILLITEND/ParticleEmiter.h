#pragma once
#include "Graphics\StructBuffer.h"
#include "Graphics\CBuffer.h"
#include "Particle.h"
struct ParticleSimCB
{
	float dt_;
	float time_;
	int aliveInCount_;
	int deadInCount_;
	int emitCount_;
	XMFLOAT4 emitterPos_;
	int maxParticles_;
	float pad_[3];
};

struct ParticleRenderCB{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	float particleSize;
	int aliveCount;
	float pad[2];
};
class ParticleEmiter {
private:
	// x - contains append amount, y - contains consume amount 
	std::shared_ptr<Cbuffer<ParticleSimCB>> emiter_cb_;
	std::shared_ptr<StructBuffer<Particle>> append; // here existing particles
	std::shared_ptr<StructBuffer<Particle>> consume;// here dead ones
	XMFLOAT4 position;

public:
	ParticleEmiter(std::shared_ptr<Gdevice> device, XMFLOAT4 pos=XMFLOAT4(0,0,0,1));
	std::shared_ptr<StructBuffer<Particle>> GetAppend() { return append; }
	std::shared_ptr<StructBuffer<Particle>> GetConsume() { return consume; }
	void UpdateCbuffer(float dt, float time, int emitCount);
};
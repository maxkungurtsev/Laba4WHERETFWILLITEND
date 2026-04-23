#pragma once
#include "Graphics\StructBuffer.h"
#include "Graphics\CBuffer.h"
#include "Graphics\GTexture.h"
#include <DirectXTex.h>
#include "Particle.h"
struct ParticleSimCB
{
	float dt_;
	float time_;
	int aliveInCount_;
	int deadInCount_;
	int emitCount_;
	XMFLOAT4 emiter_pos_;
	float pad_[2];
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
	std::shared_ptr<GTexture> texture;
	// x - contains append amount, y - contains consume amount 
	std::shared_ptr<Cbuffer<ParticleSimCB>> emiter_cb_;
	std::shared_ptr<Cbuffer<ParticleRenderCB>> emiter_render_cb_;
	std::shared_ptr<StructBuffer<Particle>> append; // here existing particles
	std::shared_ptr<StructBuffer<Particle>> consume;// here dead ones
	XMFLOAT4 position;

public:
	ParticleEmiter(std::string& texture_name, std::shared_ptr<Gdevice> device, XMFLOAT4 pos, float dt, float time, int max_particles, int emit_count);
	std::shared_ptr<StructBuffer<Particle>> GetAppend() { return append; }
	std::shared_ptr<StructBuffer<Particle>> GetConsume() { return consume; }
	void UpdateCbuffer(float time);
	std::shared_ptr<Cbuffer<ParticleSimCB>> GetParticleSimCB() { return emiter_cb_; };
	std::shared_ptr<Cbuffer<ParticleRenderCB>> GetParticleRenderCB() { return emiter_render_cb_; };
	std::shared_ptr<GTexture> GetTexture() { return texture; }
};
#pragma once
#include "Graphics\StructBuffer.h"
#include "Graphics\CBuffer.h"
#include "Graphics\GTexture.h"
#include <DirectXTex.h>
#include "Particle.h"
#include <algorithm>
#include <random>
struct ParticleSimCB
{
	XMFLOAT4 emiter_pos_;
	float dt_;
	float time_;
	int aliveInCount_;
	int deadInCount_;
	int emitCount_;
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
	std::shared_ptr<GTexture> texture;
	// x - contains append amount, y - contains consume amount 
	std::shared_ptr<Cbuffer<ParticleSimCB>> emiter_cb_;
	std::shared_ptr<Cbuffer<ParticleRenderCB>> emiter_render_cb_;
	std::shared_ptr<StructBuffer<Particle>> alive_in_; // here existing particles
	std::shared_ptr<StructBuffer<Particle>> alive_out_;// here dead ones
	std::shared_ptr<StructBuffer<Particle>> dead_in_; // here existing particles
	std::shared_ptr<StructBuffer<Particle>> dead_out_;// here dead ones
	XMFLOAT4 position;
	int emit_count_;

public:
	ParticleEmiter(std::string& texture_name, std::shared_ptr<Gdevice> device, XMFLOAT4 pos, float dt, float time, int max_particles, int emit_count, float part_size);
	void SwapSimulationBuffers();
	void UpdateCbuffer(float time);
	std::shared_ptr<StructBuffer<Particle>> GetAliveIn() { return alive_in_; }
	std::shared_ptr<StructBuffer<Particle>> GetAliveOut() { return alive_out_; }
	std::shared_ptr<StructBuffer<Particle>> GetDeadIn() { return dead_in_; }
	std::shared_ptr<StructBuffer<Particle>> GetDeadOut() { return dead_out_; }
	std::shared_ptr<Cbuffer<ParticleSimCB>> GetParticleSimCB() { return emiter_cb_; };
	std::shared_ptr<Cbuffer<ParticleRenderCB>> GetParticleRenderCB() { return emiter_render_cb_; };
	std::shared_ptr<GTexture> GetTexture() { return texture; }
};
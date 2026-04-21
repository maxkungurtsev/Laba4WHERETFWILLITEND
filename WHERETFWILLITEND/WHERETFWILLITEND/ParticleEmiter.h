#pragma once
#include "Graphics\StructBuffer.h"
#include "Graphics\CBuffer.h"
#include "Particle.h"
class ParticleEmiter {
private:
	// x - contains append amount, y - contains consume amount 
	std::shared_ptr <Cbuffer<XMFLOAT4>> elems_in_append_consume;
	std::shared_ptr<StructBuffer<Particle>> append; // here existing particles
	std::shared_ptr<StructBuffer<Particle>> consume;// here dead ones
	XMFLOAT4 position;

public:
	ParticleEmiter(std::shared_ptr<Gdevice> device, XMFLOAT4 pos=XMFLOAT4(0,0,0,1));
	void AddParticleToAppend(Particle& particle, bool in_render_frame);
	void AddParticleToConsume(Particle& particle, bool in_render_frame);
	void RemoveLastFromAppend(bool in_render_frame);
	void RemoveLastFromConsume(bool in_render_frame);
	std::shared_ptr<StructBuffer<Particle>> GetAppend() { return append; }
	std::shared_ptr<StructBuffer<Particle>> GetConsume() { return consume; }
};
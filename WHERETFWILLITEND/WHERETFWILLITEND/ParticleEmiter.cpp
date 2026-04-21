#pragma once
#include "ParticleEmiter.h"
ParticleEmiter::ParticleEmiter(XMFLOAT4 pos, std::shared_ptr<Gdevice> device) {
	position = pos;
	UINT max_particles = 100;
	append = std::make_shared<StructBuffer<Particle>>(device, 100);
	consume = std::make_shared<StructBuffer<Particle>>(device, 100);
}
void ParticleEmiter::AddParticleToAppend(Particle& particle, bool in_render_frame) {
	elems_in_append++;
	append->AddElement(particle, elems_in_append, in_render_frame);
};
void ParticleEmiter::AddParticleToConsume(Particle& particle, bool in_render_frame) {
	elems_in_consume++;
	consume->AddElement(particle, elems_in_consume, in_render_frame);
};
void ParticleEmiter::RemoveLastFromAppend(bool in_render_frame) {
	append->RemoveLastElement(in_render_frame);
};
void ParticleEmiter::RemoveLastFromConsume(bool in_render_frame) {
	consume->RemoveLastElement(in_render_frame);
};
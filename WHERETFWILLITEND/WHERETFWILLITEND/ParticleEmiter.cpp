#pragma once
#include "ParticleEmiter.h"
ParticleEmiter::ParticleEmiter(std::shared_ptr<Gdevice> device, XMFLOAT4 pos) {
	position = pos;
	UINT max_particles = 100;
	append = std::make_shared<StructBuffer<Particle>>(device, 100);
	consume = std::make_shared<StructBuffer<Particle>>(device, 100);
	elems_in_append_consume = std::make_shared<Cbuffer<XMFLOAT4>>(device);
	elems_in_append_consume->GetData() = XMFLOAT4{ 0,0,0,0 };
}
void ParticleEmiter::AddParticleToAppend(Particle& particle, bool in_render_frame) {
	elems_in_append_consume->GetData().x++;
	append->AddElement(particle, elems_in_append, in_render_frame);
};
void ParticleEmiter::AddParticleToConsume(Particle& particle, bool in_render_frame) {
	elems_in_append_consume->GetData().y++;
	consume->AddElement(particle, elems_in_consume, in_render_frame);
};
void ParticleEmiter::RemoveLastFromAppend(bool in_render_frame) {
	elems_in_append_consume->GetData().x--;
	append->RemoveLastElement(in_render_frame);
};
void ParticleEmiter::RemoveLastFromConsume(bool in_render_frame) {
	elems_in_append_consume->GetData().y--;
	consume->RemoveLastElement(in_render_frame);
};
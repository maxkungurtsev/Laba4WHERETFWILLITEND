#pragma once
#include "ParticleEmiter.h"
ParticleEmiter::ParticleEmiter(std::shared_ptr<Gdevice> device, XMFLOAT4 pos, float dt, float time, int emitCount) {
	position = pos;
	UINT max_particles = 100;
	append = std::make_shared<StructBuffer<Particle>>(device, 100);
	consume = std::make_shared<StructBuffer<Particle>>(device, 100);
	emiter_cb_ = std::make_shared<Cbuffer<XMFLOAT4>>(device);
	UpdateCbuffer(dt, time, emitCount);
	emiter_cb_->GetData().dt_ = dt;
	emiter_cb_->GetData().maxParticles_ = 1000;
	emiter_cb_->GetData().emitterPos = pos;
}
void UpdateCbuffer(float dt, float time, int emitCount) {
	emiter_cb_->GetData().dt_ = dt;
	emiter_cb_->GetData().time_ = time;
	emiter_cb_->GetData().emitCount_ = emitCount;
	emiter_cb_->GetData().aliveInCount_ = dt;
	emiter_cb_->GetData().dt_ = deadInCount;
};

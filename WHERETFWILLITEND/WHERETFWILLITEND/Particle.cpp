#include "Particle.h"

Particle::Particle(std::shared_ptr<Gdevice> device, XMFLOAT3 initial_pos) {
	remaining_life_ = rand() * 10;
	position_ = initial_pos;
	velocity_ = XMFLOAT3(rand() * 10, rand() * 10, rand() * 10);
}
Particle::Particle(Particle& particle) {
	remaining_life_ = particle->GetLife();
	position_ = particle->GetPos();
	velocity_ = particle->GetVelocity();
}
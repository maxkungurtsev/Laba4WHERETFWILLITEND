#include "Particle.h"

Particle::Particle() {
	remaining_life_ = 0.0f;
	position_ = XMFLOAT4(0,0,0,1);
	velocity_ = XMFLOAT3(0, 0, 0);
}
void Particle::SetPos(XMFLOAT4 pos) {
	position_ = pos;
};
void Particle::SetVelocity(XMFLOAT3 vel) {
	velocity_ = vel;
}

void Particle::SetLife(float life) {
	remaining_life_ = life;
}
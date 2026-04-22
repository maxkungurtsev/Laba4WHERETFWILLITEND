#include "Particle.h"

Particle::Particle() {
	remaining_life_ = rand() * 10;
	position_ = XMFLOAT4(0,0,0,1);
	velocity_ = XMFLOAT3(rand() * 10, rand() * 10, rand() * 10);
}
void Particle::SetPos(XMFLOAT4 pos) {
	position_ = pos;
};

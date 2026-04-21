#pragma once
#include <DirectXMath.h>
#include "Graphics\GDevice.h"
using namespace DirectX;
class Particle {
private:
	XMFLOAT4 position_;
	float remaining_life_;
	XMFLOAT3 velocity_;
public:
	XMFLOAT3& GetVelocity() { return velocity_; }
	float GetLife() { return remaining_life_; }
	XMFLOAT4& GetPos() { return position_; }
	Particle(std::shared_ptr<Gdevice> device, XMFLOAT3 initial_pos);
	Particle(Particle& particle);
};
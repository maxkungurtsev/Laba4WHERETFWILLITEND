#include "Lights.h"
Lights::Lights(std::shared_ptr<Gdevice> device) {
	lights_ =std::make_unique<StructBuffer<LightData>>(device, 200);
}


void Lights::Addlight(XMFLOAT3 strength, float falloff_start, XMFLOAT4 direction, XMFLOAT4 position, float falloff_end, float spot_power, int type, float velocity) {
	LightData newlight;
	newlight.direction = direction;
	newlight.strength = strength;
	newlight.falloff_start = falloff_start;
	newlight.position = position;
	newlight.falloff_end = falloff_end;
	newlight.spot_power = spot_power;
	newlight.type = type;
	newlight.velocity = velocity;
	lights_->AddElement(newlight);
};
void Lights::AddAmbientlight(XMFLOAT3 strength) {
	LightData newlight;
	newlight.strength = strength;
	newlight.type = 3;
	lights_->AddElement(newlight);
}
void Lights::RemoveLastLight() {
	lights_->RemoveLastElement();
}
std::shared_ptr<StructBuffer<LightData>> Lights::GetBuffer() {
	return lights_;
}
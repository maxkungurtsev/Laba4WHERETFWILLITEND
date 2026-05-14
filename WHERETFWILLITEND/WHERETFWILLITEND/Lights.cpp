#include "Lights.h"
Lights::Lights(std::shared_ptr<Gdevice> device) {
	lights_ =std::make_shared<StructBuffer<LightData>>(device, 200, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	max_lights_ = std::make_shared<Cbuffer<XMFLOAT4>>(device);
	max_lights_->GetData() = XMFLOAT4{0,0,0,0};
	device_ = device;
}


void Lights::AddSpotlight(XMFLOAT3 strength, float falloff_start, XMFLOAT4 direction, XMFLOAT4 position, float falloff_end, float spot_power, bool in_render_frame, float velocity, float spawn_time, XMFLOAT4 movement_direction) {
	LightData newlight;
	newlight.direction = direction;
	newlight.strength = strength;
	newlight.falloff_start = falloff_start;
	newlight.position = position;
	newlight.falloff_end = falloff_end;
	newlight.spot_power = spot_power;
	newlight.type = 2;
	newlight.velocity = velocity;
	newlight.spawn_time=spawn_time;
	newlight.movement_direction = movement_direction;
	//newlight.shad_map_index = shad_maps_.size();
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
	// shad map init
	UINT cascades_amount=4;
	//sm[0] = std::make_shared<ShadowMap>(1024, 1024, cascades_amount, device_);
	//shad_maps_.push_back(sm);
};
void Lights::AddAmbientlight(XMFLOAT3 strength, bool in_render_frame) {
	LightData newlight;
	newlight.direction = {0,0,0,0};
	newlight.strength = strength;
	newlight.falloff_start = 0;
	newlight.position = { 0,0,0,0 };
	newlight.falloff_end = 0;
	newlight.spot_power = 0;
	newlight.type = 3;
	newlight.velocity = 0;
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
}

void Lights::AddDirlight(XMFLOAT3 strength, XMFLOAT4 direction, bool in_render_frame) {
	LightData newlight;
	newlight.direction = direction;
	newlight.strength = strength;
	newlight.falloff_start = 0;
	newlight.position = { 0,0,0,0 };
	newlight.falloff_end = 0;
	newlight.spot_power = 0;
	newlight.type = 0;
	newlight.velocity = 0;
	//newlight.shad_map_index = shad_maps_.size();
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
    // shad map init

	//sm[0] = std::make_shared<ShadowMap>(1024, 1024, 4, device_);
	//shad_maps_.push_back(sm);
}
void Lights::AddPointlight(XMFLOAT3 strength, XMFLOAT4 position, float falloff_start, float falloff_end, bool in_render_frame, float velocity, float spawn_time, XMFLOAT4 movement_direction) {
	LightData newlight;
	newlight.direction = { 0,0,0,0 };
	newlight.strength = strength;
	newlight.falloff_start = falloff_start;
	newlight.position = position;
	newlight.falloff_end = falloff_end;
	newlight.spot_power = 0;
	newlight.type = 1;
	newlight.velocity = velocity;
	newlight.spawn_time = spawn_time;
	newlight.movement_direction = movement_direction;
	//newlight.shad_map_index = shad_maps_.size();
	lights_->AddElement(newlight, max_lights_->GetData().x, in_render_frame);
	max_lights_->GetData().x += 1;
	max_lights_->Save_changes();
	// shad map init
	//sm[0] = std::make_shared<ShadowMap>(1024, 1024, 4, device_);
	//sm[1] = std::make_shared<ShadowMap>(1024, 1024, 4, device_);
	//sm[2] = std::make_shared<ShadowMap>(1024, 1024, 4, device_);
	//sm[3] = std::make_shared<ShadowMap>(1024, 1024, 4, device_);
	//sm[4] = std::make_shared<ShadowMap>(1024, 1024, 4, device_);
	//sm[5] = std::make_shared<ShadowMap>(1024, 1024, 4, device_);
	//shad_maps_.push_back(sm);
}

void Lights::RemoveLastLight(bool in_render_frame) {
	lights_->RemoveLastElement(in_render_frame);
	max_lights_->GetData().x -= 1;
	max_lights_->Save_changes();
}
std::shared_ptr<StructBuffer<LightData>> Lights::GetBuffer() {
	return lights_;
}
std::shared_ptr <Cbuffer<XMFLOAT4>> Lights::GetMaxLights() {
	return max_lights_;
}
